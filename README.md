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
