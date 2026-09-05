CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS := -Iinclude -D_POSIX_C_SOURCE=200809L

TARGET := minishell

SOURCES := \
	src/main.c \
	src/builtins.c \
	src/executor.c \
	src/input.c \
	src/lexer.c \
	src/parser.c \
	src/signals.c

LEXER_TEST := test_lexer
PARSER_TEST := test_parser
EXECUTOR_TEST := test_executor
REDIRECTION_TEST := test_redirection
PIPELINE_TEST := test_pipeline
INPUT_TEST := test_input

TEST_TARGETS := \
	$(INPUT_TEST) \
	$(LEXER_TEST) \
	$(PARSER_TEST) \
	$(EXECUTOR_TEST) \
	$(REDIRECTION_TEST) \
	$(PIPELINE_TEST)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/shell.h include/input.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(LEXER_TEST): tests/test_lexer.c src/lexer.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_lexer.c src/lexer.c \
		-o $(LEXER_TEST)

$(PARSER_TEST): tests/test_parser.c src/lexer.c src/parser.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_parser.c src/lexer.c src/parser.c \
		-o $(PARSER_TEST)

$(EXECUTOR_TEST): tests/test_executor.c src/executor.c src/signals.c include/shell.h include/signals.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_executor.c src/executor.c src/signals.c \
		-o $(EXECUTOR_TEST)

$(REDIRECTION_TEST): tests/test_redirection.c src/executor.c src/signals.c include/shell.h include/signals.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_redirection.c \
		src/executor.c \
		src/signals.c \
		-o $(REDIRECTION_TEST)

$(PIPELINE_TEST): tests/test_pipeline.c src/lexer.c src/parser.c src/executor.c src/signals.c include/shell.h include/signals.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_pipeline.c \
		src/lexer.c \
		src/parser.c \
		src/executor.c \
		src/signals.c \
		-o $(PIPELINE_TEST)

$(INPUT_TEST): tests/test_input.c src/input.c include/input.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		tests/test_input.c src/input.c \
		-o $(INPUT_TEST)

test: $(TEST_TARGETS)
	./$(INPUT_TEST)
	./$(LEXER_TEST)
	./$(PARSER_TEST)
	./$(EXECUTOR_TEST)
	./$(REDIRECTION_TEST)
	./$(PIPELINE_TEST)

clean:
	rm -f $(TARGET) $(TEST_TARGETS)
