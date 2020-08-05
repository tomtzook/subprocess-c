#pragma once

#include <stddef.h>

#define SUBPROCESS_WAIT_EXIT_NORMAL (0)
#define SUBPROCESS_WAIT_EXIT_SIGNAL (1)
#define SUBPROCESS_WAIT_NO_CHILD (2)

typedef enum {
    SUBPROCESS_PIPE_NONE = 0,
    SUBPROCESS_PIPE_NORMAL
} subprocess_pipe_t;

typedef enum {
    SUBPROCESS_SHAREDMEM_NONE = 0,
    SUBPROCESS_SHAREDMEM_ANONYMOUS
} subprocess_sharedmem_t;

typedef struct {
    subprocess_pipe_t stdin_pipe;
    subprocess_pipe_t stdout_pipe;
    subprocess_pipe_t stderr_pipe;

    char* path;
    char** argv;
    char** envp;
} subprocess_def_t;

typedef struct {
    void* sharedmem;
    size_t sharedmem_size;

    void* param;
    size_t param_size;
} subprocess_func_ctx_t;

typedef int (*subprocess_main_t)(const subprocess_func_ctx_t*);

typedef struct {
    subprocess_pipe_t stdin_pipe;
    subprocess_pipe_t stdout_pipe;
    subprocess_pipe_t stderr_pipe;

    subprocess_sharedmem_t sharedmem;
    size_t sharedmem_size;

    subprocess_main_t entry_point;
    void* param;
    size_t param_size;
} subprocess_func_t;

typedef struct {
    pid_t pid;

    void* sharedmem;
    size_t sharedmem_size;

    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
} subprocess_run_t;

int subprocess_create(const subprocess_def_t* proc_def,
        subprocess_run_t* proc_run);
int subprocess_create_func(const subprocess_func_t* proc_def,
        subprocess_run_t* proc_run);
void subprocess_free(subprocess_run_t* proc_run);

int subprocess_communicate(subprocess_run_t* proc_run,
        const void* input, size_t input_size,
        void* output, size_t output_size, size_t* output_read,
        void* error, size_t error_size, size_t* error_read);

int subprocess_kill(const subprocess_run_t* proc_run, int signal);

int subprocess_wait(const subprocess_run_t* proc_run, int* exit_code);

void subprocess_close_pipe(int* fd_ptr);
