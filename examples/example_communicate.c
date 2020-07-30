#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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

    char input[64] = "hello\nworld\nhey hello";
    char output[64];
    size_t output_written;
    char error[64];
    size_t error_written;

    result = subprocess_communicate(&proc,
                input, strlen(input),
                output, 64, &output_written,
                error, 64, &error_written);
    if (result) {
        printf("Error communicate %d, %d\n", result, errno);
        // incase nothing was written, so we can make grep finish normally
        close(proc.stdin_fd);
    } else {
        printf("stdout: %.*s", (int) output_written, output);
        printf("stderr: %.*s", (int) error_written, error);
        printf("\n");
    }

    int exit_code;
    result = subprocess_wait(&proc, &exit_code);
    if (result < 0) {
        printf("Error waiting for subprocess %d\n", result);
    } else {
        printf("Done waiting for subprocess %d, exit code %d\n", result, exit_code);
    }

    subprocess_free(&proc);
    return 0;
}





