#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../subprocess.h"

int main(int argc, char** argv) {
    char* argv_[] = {"", "hello", NULL};
    char* envp_[] = {NULL};

    subprocess_def_t def = {0};
    def.path = "/bin/grep";
    def.argv = argv_;
    def.envp = envp_;
    def.stdin_pipe = SUBPROCESS_PIPE_NORMAL;
    def.stdout_pipe = SUBPROCESS_PIPE_NORMAL;
    def.stderr_pipe = SUBPROCESS_PIPE_NORMAL;

    subprocess_run_t proc;

    int result = subprocess_create(&def, &proc);
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
        printf("Error communicate %d\n", result);
        // in case nothing was written, so we can make grep finish normally
        subprocess_close_pipe(&proc.stdin_fd);
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





