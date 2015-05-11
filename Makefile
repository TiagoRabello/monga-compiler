###############################################################################
# Makefile for Monga Lexical Analyzer
#
# Author: Tiago Gomes
#
###############################################################################

###############################################################################
# Working directories
###############################################################################
AST_DIR=src/ast
BIN_DIR=bin
SCANNER_DIR=src/scanner
PARSER_DIR=src/parser
TEST_INPUT_DIR=test/inputs
TEST_ANSWERS_DIR=test/answers

###############################################################################
# Costumizable programs
###############################################################################
LEX=flex
YACC=bison
DIFF=diff
#CC=gcc

###############################################################################
# Test file names
###############################################################################
POSITIVE_TEST_NAMES = hello_world fibonacci commented_out operators program_inside_string empty_block array empty_return
NEGATIVE_TEST_NAMES = global_statement single_line_decl_def no_name_param decl_var_mid_block
TEST_NAMES = $(POSITIVE_TEST_NAMES) $(NEGATIVE_TEST_NAMES)

###############################################################################
# Test printing definitions
###############################################################################
ECHO_RED=\033[0;31m
ECHO_GREEN=\033[0;32m
ECHO_NORMAL=\033[0m

SUCCESS_MSG="${ECHO_GREEN}Passed!${ECHO_NORMAL}"
FAILURE_MSG="${ECHO_RED}Failed!${ECHO_NORMAL}"
PRINT_RESULT_MSG=/usr/bin/test "$$?" -eq 0 && echo -e $(SUCCESS_MSG) || echo -e $(FAILURE_MSG)

###############################################################################
# Targets
###############################################################################
test: test_scanner test_parser

# Run every lexical analyzer test.
test_scanner: scanner_test
	@echo "Starting scanner tests:"
	@$(foreach name, $(TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > out && $(DIFF) out $(TEST_ANSWERS_DIR)/$(name).scanner.answer; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@rm out

# Run every parser test.
test_parser: parser_test
	@echo "Starting positive parser tests:"
	@$(foreach name, $(POSITIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > out && $(DIFF) out $(TEST_ANSWERS_DIR)/$(name).parser.answer; \
		/usr/bin/test "$$?" -eq 0; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@echo "Starting negative parser tests:"
	@$(foreach name, $(NEGATIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > /dev/null 2> /dev/null ; \
		/usr/bin/test "$$?" -eq 1; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@rm out

# Build lexical analyzer's object files.
monga_scanner: $(BIN_DIR)/monga_scanner.o $(BIN_DIR)/monga_parser.o
	$(CC) -o $(BIN_DIR)/$@ $^

# Remove all generated files.
clean:
	rm -f out $(SCANNER_DIR)/monga_scanner.c \
	          $(SCANNER_DIR)/monga_tokens.h \
	          $(PARSER_DIR)/monga_parser.c \
	          $(BIN_DIR)/*

scanner_test: $(BIN_DIR)/monga_scanner_debug.o $(BIN_DIR)/monga_parser.o $(BIN_DIR)/scanner_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

parser_test: $(BIN_DIR)/monga_scanner.o $(BIN_DIR)/monga_parser.o $(BIN_DIR)/ast_printer.o $(BIN_DIR)/parser_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

# AST related files.
$(BIN_DIR)/ast_printer.o: $(AST_DIR)/ast_printer.c $(AST_DIR)/ast_printer.h $(AST_DIR)/ast.h
	$(CC) -c -o $@ $<

# Scanner related files.

$(SCANNER_DIR)/monga_scanner.c: $(SCANNER_DIR)/monga.lex
	$(LEX) -o $@ $<

$(BIN_DIR)/monga_scanner.o: $(SCANNER_DIR)/monga_scanner.c $(SCANNER_DIR)/monga_tokens.h
	$(CC) -c -o $@ $<

$(BIN_DIR)/monga_scanner_debug.o: $(SCANNER_DIR)/monga_scanner.c $(SCANNER_DIR)/monga_tokens.h
	$(CC) -DSCANNER_DEBUG -c -o $@ $<

$(BIN_DIR)/scanner_test_main.o: $(SCANNER_DIR)/test.c
	$(CC) -c -o $@ $<

# Parser related files.

$(PARSER_DIR)/monga_parser.c $(SCANNER_DIR)/monga_tokens.h: $(PARSER_DIR)/monga.y
	$(YACC) -o $(PARSER_DIR)/monga_parser.c --defines=$(SCANNER_DIR)/monga_tokens.h $<

$(BIN_DIR)/monga_parser.o: $(PARSER_DIR)/monga_parser.c
	$(CC) -c -o $@ $<

$(BIN_DIR)/parser_test_main.o: $(PARSER_DIR)/test.c $(PARSER_DIR)/monga_parser.h
	$(CC) -c -o $@ $<