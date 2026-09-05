#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#include "signals.h"


static void handle_sigint(int signal_number)
{
    (void)signal_number;

    /*
     * write() is safe inside a signal handler.
     * printf() is not.
     */
    const char newline = '\n';
    write(STDOUT_FILENO, &newline, 1);
}

void install_shell_signal_handlers(void)
{
    struct sigaction action = {0};

    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);

    /*
     * Prevent Ctrl+\ from terminating the shell.
     */
    action.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &action, NULL);
}

void restore_child_signal_handlers(void)
{
    struct sigaction action = {0};

    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
}

