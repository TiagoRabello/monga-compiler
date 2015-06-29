###############################################################################
# Makefile for Monga Lexical Analyzer
#
# Author: Tiago Gomes
#
###############################################################################

###############################################################################
# Working directories
###############################################################################
BIN_DIR=bin

AST_DIR=src/ast
BACKEND_DIR=src/backend
COMMON_DIR=src/common
PARSER_DIR=src/parser
SCANNER_DIR=src/scanner
SEMANTIC_DIR=src/semantic
SRC_DIR=src

TEST_INPUT_DIR=test/inputs
TEST_ANSWERS_DIR=test/answers

###############################################################################
# Costumizable programs
###############################################################################
LEX=flex
YACC=bison
DIFF=diff
CC=gcc

###############################################################################
# Test file names
###############################################################################
BACKEND_POSITIVE_TEST_NAMES = array \
                              fibonacci \
                              operators \
                              program_inside_string \
                              sorts

SEMANTIC_POSITIVE_TEST_NAMES = ${BACKEND_POSITIVE_TEST_NAMES} \
                               commented_out \
                               empty_block \
                               empty_return \
                               use_var_inner_scope
SEMANTIC_NEGATIVE_TEST_NAMES = assign_type_mismatch \
                               call_func_before_decl \
                               decl_void_var \
                               hello_world \
                               index_non_indexable \
                               no_return_non_void_func \
                               non_int_if_condition \
                               non_int_while_condition \
                               too_few_args \
                               too_many_args \
                               type_mismatch_args \
                               type_mismatch_return \
                               use_undefined_var \
                               use_var_outside_scope

PARSER_POSITIVE_TEST_NAMES = ${SEMANTIC_POSITIVE_TEST_NAMES} ${SEMANTIC_NEGATIVE_TEST_NAMES}
PARSER_NEGATIVE_TEST_NAMES = global_statement single_line_decl_def no_name_param decl_var_mid_block

TEST_NAMES = $(PARSER_POSITIVE_TEST_NAMES) $(PARSER_NEGATIVE_TEST_NAMES)

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
test: test_scanner test_parser test_semantic test_backend

# Run every backend test.
test_backend: backend_test
	@echo "Starting backend tests:"
	@$(foreach name, $(BACKEND_POSITIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > out && $(DIFF) out $(TEST_ANSWERS_DIR)/$(name).backend.answer; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@rm out

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
	@$(foreach name, $(PARSER_POSITIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > out && $(DIFF) out $(TEST_ANSWERS_DIR)/$(name).parser.answer; \
		/usr/bin/test "$$?" -eq 0; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@echo "Starting negative parser tests:"
	@$(foreach name, $(PARSER_NEGATIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > /dev/null 2> /dev/null ; \
		/usr/bin/test "$$?" -eq 1; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@rm out

# Run every semantic test.
test_semantic: semantic_test
	@echo "Starting positive semantic tests:"
	@$(foreach name, $(SEMANTIC_POSITIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > /dev/null; \
		/usr/bin/test "$$?" -eq 0; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"
	@echo "Starting negative semantic tests:"
	@$(foreach name, $(SEMANTIC_NEGATIVE_TEST_NAMES), \
		echo -n "- Testing input $(name).monga..."; \
		$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/$(name).monga > /dev/null 2> /dev/null ; \
		/usr/bin/test "$$?" -eq 1; \
		$(PRINT_RESULT_MSG);)
	@echo "Done!"

# Remove all generated files.
clean:
	rm -f out $(SCANNER_DIR)/monga_scanner.c \
	          $(SCANNER_DIR)/monga_tokens.h \
	          $(PARSER_DIR)/monga_parser.c \
	          $(BIN_DIR)/*

scanner_test: $(BIN_DIR)/monga_scanner_debug.o \
	            $(BIN_DIR)/monga_parser.o \
	            $(BIN_DIR)/scanner_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

parser_test: $(BIN_DIR)/monga_scanner.o \
	           $(BIN_DIR)/monga_parser.o \
	           $(BIN_DIR)/ast_printer.o \
	           $(BIN_DIR)/parser_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

semantic_test: $(BIN_DIR)/monga_scanner.o \
               $(BIN_DIR)/monga_parser.o \
               $(BIN_DIR)/ast_printer.o \
               $(BIN_DIR)/resolve_ids.o \
               $(BIN_DIR)/resolve_types.o \
               $(BIN_DIR)/semantic_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

backend_test: $(BIN_DIR)/monga_scanner.o \
              $(BIN_DIR)/monga_parser.o \
              $(BIN_DIR)/ast_printer.o \
              $(BIN_DIR)/resolve_ids.o \
              $(BIN_DIR)/resolve_types.o \
              $(BIN_DIR)/ia32_assembly.o \
              $(BIN_DIR)/backend_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

compiler: $(BIN_DIR)/monga_scanner.o \
          $(BIN_DIR)/monga_parser.o \
          $(BIN_DIR)/ast_printer.o \
          $(BIN_DIR)/resolve_ids.o \
          $(BIN_DIR)/resolve_types.o \
          $(BIN_DIR)/ia32_assembly.o \
          $(BIN_DIR)/compiler_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

# Compiler related files.
compile: $(BIN_DIR)/compiler
	@$^ < $(src) > out.s && $(CC) -c out.s -o out.o && $(CC) src/runtime/main.c out.o -o out.exe

$(BIN_DIR)/compiler: compiler

$(BIN_DIR)/compiler_main.o: $(SRC_DIR)/main.c
	$(CC) -c -o $@ $<

# AST related files.
$(BIN_DIR)/ast_printer.o: $(AST_DIR)/ast_printer.c $(AST_DIR)/ast_printer.h $(AST_DIR)/ast.h
	$(CC) -c -o $@ $<

# Backend related files.
$(BIN_DIR)/ia32_assembly.o: $(BACKEND_DIR)/ia32_assembly.c \
                            $(BACKEND_DIR)/ia32_assembly.h \
                            $(AST_DIR)/ast.h \
                            $(AST_DIR)/ast_printer.h \
                            $(COMMON_DIR)/diagnostics.h
	$(CC) -c -o $@ $<

$(BIN_DIR)/backend_test_main.o: $(BACKEND_DIR)/test.c
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

# Semantics related files.

$(BIN_DIR)/resolve_ids.o: $(SEMANTIC_DIR)/resolve_ids.c \
                          $(SEMANTIC_DIR)/semantic.h \
                          $(AST_DIR)/ast.h \
                          $(AST_DIR)/ast_printer.h \
                          $(COMMON_DIR)/diagnostics.h
	$(CC) -c -o $@ $<

$(BIN_DIR)/resolve_types.o: $(SEMANTIC_DIR)/resolve_types.c \
                            $(SEMANTIC_DIR)/semantic.h \
                            $(AST_DIR)/ast.h \
                            $(AST_DIR)/ast_printer.h \
                            $(COMMON_DIR)/diagnostics.h
	$(CC) -c -o $@ $<

$(BIN_DIR)/semantic_test_main.o: $(SEMANTIC_DIR)/test.c \
	                               $(SEMANTIC_DIR)/semantic.h \
	                               $(AST_DIR)/ast.h \
	                               $(PARSER_DIR)/monga_parser.h
	$(CC) -c -o $@ $<