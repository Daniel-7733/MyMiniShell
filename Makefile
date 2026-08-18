CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS := -Iinclude

TARGET := minishell
SOURCES := src/main.c src/shell.c
TEST_TARGET := test_tokenize

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TEST_TARGET): tests/test_tokenize.c src/shell.c include/shell.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_tokenize.c src/shell.c -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

