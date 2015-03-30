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
SCANNER_DIR=scanner
TEST_INPUT_DIR=test_inputs
TEST_ANSWERS_DIR=test_answers

###############################################################################
# Costumizable programs
###############################################################################
LEX=flex
DIFF=diff
#CC=gcc

# Run every lexical analyzer test.
test_scanner: scanner_test
	$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/hello_world.monga > out
	$(DIFF) out $(TEST_ANSWERS_DIR)/hello_world.answer
	$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/fibonacci.monga > out
	$(DIFF) out $(TEST_ANSWERS_DIR)/fibonacci.answer
	$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/commented_out.monga > out
	$(DIFF) out $(TEST_ANSWERS_DIR)/commented_out.answer
	$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/operators.monga > out
	$(DIFF) out $(TEST_ANSWERS_DIR)/operators.answer
	$(BIN_DIR)/$< < $(TEST_INPUT_DIR)/program_inside_string.monga > out
	$(DIFF) out $(TEST_ANSWERS_DIR)/program_inside_string.answer
	rm out

# Build lexical analyzer's object files.
monga_scanner: $(BIN_DIR)/monga_scanner.o

# Remove all generated files.
clean:
	rm -f out $(SCANNER_DIR)/monga_scanner.c $(BIN_DIR)/*.o $(BIN_DIR)/scanner_test_main

scanner_test: $(BIN_DIR)/monga_scanner.o $(BIN_DIR)/scanner_test_main.o
	$(CC) -o $(BIN_DIR)/$@ $^

$(SCANNER_DIR)/monga_scanner.c: $(SCANNER_DIR)/monga.lex
	$(LEX) -o $@ $<

$(BIN_DIR)/monga_scanner.o: $(SCANNER_DIR)/monga_scanner.c $(SCANNER_DIR)/monga_tokens.h
	$(CC) -c -o $@ $<

$(BIN_DIR)/scanner_test_main.o: $(SCANNER_DIR)/test.c
	$(CC) -c -o $@ $<
