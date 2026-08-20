CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS := -Iinclude

TARGET := minishell
SOURCES := src/main.c src/shell.c
TOKENIZER_TEST := test_tokenize
EXTERNAL_TEST := test_execute_external

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

test: $(TOKENIZER_TEST) $(EXTERNAL_TEST)
	./$(TOKENIZER_TEST)
	./$(EXTERNAL_TEST)

clean:
	rm -f $(TARGET) $(TOKENIZER_TEST) $(EXTERNAL_TEST)

