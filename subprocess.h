#pragma once

#include <stddef.h>

typedef int (*subprocess_main_t)(void*, size_t);

typedef enum {
    NONE = 0,
    PIPE
} subprocess_pipe_t;

typedef struct {
    subprocess_main_t entry_point;
    void* param;
    size_t param_size;

    subprocess_pipe_t stdin_pipe;
    subprocess_pipe_t stdout_pipe;
    subprocess_pipe_t stderr_pipe;
} subprocess_def_t;

typedef struct {
    int pid;

    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
} subprocess_run_t;

int subprocess_create(const subprocess_def_t* proc_def,
        subprocess_run_t* proc_run);
void subprocess_free(subprocess_run_t* proc_run);

int subprocess_wait(const subprocess_run_t* proc_run, int* exit_code);
