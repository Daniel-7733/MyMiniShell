# MyMiniShell

MyMiniShell is a small Linux shell written in C as a learning project. Its goal
is to build an understanding of input handling, parsing, built-in commands,
processes, system calls, pipes, and redirection one stage at a time.

## Learning roadmap

1. Build the REPL loop: prompt, read, repeat, and `exit`.
2. Parse input into a command and arguments.
3. Add built-in commands such as `help`, `pwd`, and `cd`.
4. Run external programs using Linux process APIs.
5. Add pipes.
6. Add input and output redirection.

Each stage should be understood and tested before moving to the next one.

## Current stage: REPL loop

The first version can:

- display the `minishell>` prompt;
- read one line safely with `fgets`;
- remove the trailing newline;
- exit when the user enters `exit`;
- stop cleanly when it reaches end-of-file (`Ctrl+D`).

It does not execute external commands yet. That is intentional.


## Current features

MyMiniShell currently supports:

- Interactive REPL prompt
- Command tokenization
- Built-in commands:
  - `help`
  - `exit`
  - `pwd`
  - `cd`
- External command execution using `fork()`, `execvp()`, and `waitpid()`
- Child-process exit status handling
- Output redirection:
  - `>` — create or truncate a file
  - `>>` — append to a file
- Input redirection with `<`
- Combined input and output redirection
- Pipelines containing multiple external commands
- Automated tests for tokenization, external commands, redirection, and pipelines

## Examples

```bash
./minishell
```

```text
minishell> pwd
/home/user/MyMiniShell

minishell> echo hello
hello

minishell> echo hello > output.txt
minishell> cat output.txt
hello

minishell> echo second >> output.txt

minishell> sort < unsorted.txt > sorted.txt

minishell> ls | grep test | wc -l

minishell> exit
```

## Current limitations

MyMiniShell is a learning project and is not intended to replace a production shell.

The current parser:

- Requires spaces around operators such as `|`, `<`, `>`, and `>>`
- Does not yet understand single or double quotes
- Does not support escape sequences
- Does not support environment-variable expansion such as `$HOME`
- Does not support wildcard expansion such as `*.c`
- Does not yet combine redirection with pipelines
- Supports built-ins only outside pipelines
- Does not include job control or background execution with `&`

## Tests

Build and run all tests with:

```bash
make clean test
```

The test suite currently covers:

- Tokenization
- External command execution and exit statuses
- Input, output, append, and combined redirection
- Single and multiple-command pipelines

Some negative tests intentionally print error messages while confirming that invalid commands return the expected status.


## Project structure

```text
MyMiniShell/
├── include/       # Header files when the project grows
├── src/
│   └── main.c
├── tests/         # Tests added with later stages
├── .gitignore
├── Makefile
└── README.md
```

## Build and run

On Ubuntu, install the compiler tools once:

```bash
sudo apt update
sudo apt install build-essential git
```

Then build and run:

```bash
make
./minishell
```

Or use:

```bash
make run
```

Remove the compiled program with:

```bash
make clean
```

## First manual checks

```text
minishell> hello
minishell> exit
```

`hello` is only read at this stage; it is not executed. The second line exits
the shell.

