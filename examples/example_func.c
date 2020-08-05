#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>

#include "../subprocess.h"


int in_new_proc(const subprocess_func_ctx_t* ctx) {
    char* line;
    size_t len = 0;
    getline(&line, &len, stdin);

    fprintf(stdout, "%s", line);
    fprintf(stdout, "%s || %ld \n", (char*)ctx->param, ctx->param_size);

    free(line);

    return 0;
}

int main(int argc, char** argv) {
    char buffer[64];

    subprocess_func_t def = {
            .entry_point = in_new_proc,
            .param = "hello world",
            .param_size = 12,
            .stdin_pipe = SUBPROCESS_PIPE_NORMAL,
            .stdout_pipe = SUBPROCESS_PIPE_NORMAL,
            .stderr_pipe = SUBPROCESS_PIPE_NORMAL
    };
    subprocess_run_t proc;

    int result = subprocess_create_func(&def, &proc);
    if (result) {
        printf("Error starting subprocess %d\n", result);
        return 1;
    }
    printf("child process %d\n", proc.pid);

    strcpy(buffer, "hello from stdin\n");
    write(proc.stdin_fd, buffer, strlen(buffer));

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

