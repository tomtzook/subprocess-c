#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../subprocess.h"

int main(int argc, char** argv) {
    char buffer[64];

    char* argv_[] = {"", "hello world", NULL};
    char* envp_[] = {NULL};
    subprocess_def_t def = {
            .path = "/bin/echo",
            .argv = argv_,
            .envp = envp_,
            .stdin_pipe = NONE,
            .stdout_pipe = PIPE,
            .stderr_pipe = PIPE
    };
    subprocess_run_t proc;

    int result = subprocess_create(&def, &proc);
    if (result) {
        printf("Error starting subprocess %d\n", result);
        return 1;
    }
    printf("child process %d\n", proc.pid);

    int exit_code;
    result = subprocess_wait(&proc, &exit_code);
    if (result < 0) {
        printf("Error waiting for subprocess %d\n", result);
    } else {
        printf("Done waiting for subprocess %d, exit code %d\n", result, exit_code);
    }

    int read_c = read(proc.stdout_fd, buffer, 63);
    buffer[read_c] = '\0';
    printf("stdout: %s", buffer);

    read_c = read(proc.stderr_fd, buffer, 63);
    buffer[read_c] = '\0';
    printf("stderr: %s", buffer);

    subprocess_free(&proc);
    return 0;
}

