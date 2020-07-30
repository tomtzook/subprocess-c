#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "../subprocess.h"

int main(int argc, char** argv) {
    char* argv_[] = {"", "hello", NULL};
    char* envp_[] = {NULL};
    subprocess_shell_t def = {
            .path = "/bin/grep",
            .argv = argv_,
            .envp = envp_,
            .stdin_pipe = PIPE,
            .stdout_pipe = PIPE,
            .stderr_pipe = PIPE
    };
    subprocess_run_t proc;

    int result = subprocess_create_shell(&def, &proc);
    if (result) {
        printf("Error starting subprocess %d\n", result);
        return 1;
    }
    printf("child process %d\n", proc.pid);

    result = subprocess_kill(&proc, SIGTERM);
    if (result) {
        printf("Failed to kill %d\n", result);
        // kill failed, so we can make grep finish normally
        close(proc.stdin_fd);
    }

    int exit_code;
    result = subprocess_wait(&proc, &exit_code);
    if (result < 0) {
        printf("Error waiting for subprocess %d\n", result);
    } else {
        // this time we will see SUBPROCESS_WAIT_EXIT_SIGNAL
        printf("Done waiting for subprocess %d, exit code %d\n", result, exit_code);
    }

    subprocess_free(&proc);
    return 0;
}







