CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS := -Iinclude

TARGET := minishell
SOURCES := src/main.c src/shell.c

TOKENIZER_TEST := test_tokenize
EXTERNAL_TEST := test_execute_external
REDIRECTION_TEST := test_redirection
PIPELINE_TEST := test_pipeline
LEXER_TEST := test_lexer
PARSER_TEST := test_parser

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TOKENIZER_TEST): tests/test_tokenize.c src/shell.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_tokenize.c src/shell.c -o $(TOKENIZER_TEST)

$(EXTERNAL_TEST): tests/test_execute_external.c src/shell.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_execute_external.c src/shell.c -o $(EXTERNAL_TEST)

$(REDIRECTION_TEST): tests/test_redirection.c src/shell.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_redirection.c src/shell.c -o $(REDIRECTION_TEST)

$(PIPELINE_TEST): tests/test_pipeline.c src/shell.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_pipeline.c src/shell.c -o $(PIPELINE_TEST)

$(LEXER_TEST): tests/test_lexer.c src/lexer.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c -o $(LEXER_TEST)

$(PARSER_TEST): tests/test_parser.c src/lexer.c src/parser.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_parser.c src/lexer.c src/parser.c -o $(PARSER_TEST)

test: $(TOKENIZER_TEST) $(EXTERNAL_TEST) $(REDIRECTION_TEST) $(PIPELINE_TEST)
	./$(TOKENIZER_TEST)
	./$(EXTERNAL_TEST)
	./$(REDIRECTION_TEST)
	./$(PIPELINE_TEST)
	./$(LEXER_TEST)
	./$(PARSER_TEST)

clean:
	rm -f $(TARGET) $(TOKENIZER_TEST) $(EXTERNAL_TEST) $(REDIRECTION_TEST) \
		$(PIPELINE_TEST) $(LEXER_TEST) $(PARSER_TEST)

