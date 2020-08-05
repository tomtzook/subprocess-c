#pragma once

#include <stddef.h>
#include <stdint.h>

#define SUBPROCESS_CREATE_ERROR_PIPE (1)
#define SUBPROCESS_CREATE_ERROR_FORK (2)
#define SUBPROCESS_CREATE_ERROR_SHAREDMEM (3)

#define SUBPROCESS_WAIT_EXIT_NORMAL (0)
#define SUBPROCESS_WAIT_EXIT_SIGNAL (1)
#define SUBPROCESS_WAIT_NO_CHILD (2)

#define SUBPROCESS_OPTION_PIPE_STDIN (1 << 0)
#define SUBPROCESS_OPTION_PIPE_STDOUT (1 << 1)
#define SUBPROCESS_OPTION_PIPE_STDERR (1 << 2)
#define SUBPROCESS_OPTION_SHAREDMEM_ANON (1 << 3)

typedef uint32_t subprocess_options_t;

typedef struct {
    subprocess_options_t options;

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
    subprocess_options_t options;

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

int subprocess_signal(const subprocess_run_t* proc_run, int signal);

int subprocess_wait(const subprocess_run_t* proc_run, int* exit_code);

void subprocess_close_pipe(int* fd_ptr);
