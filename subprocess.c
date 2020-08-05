#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "subprocess.h"

#define PIPE_READ (0)
#define PIPE_WRITE (1)

#define FD_STDIN (0)
#define FD_STDOUT (1)
#define FD_STDERR (2)

typedef struct {
    subprocess_pipe_t stdin_pipe;
    subprocess_pipe_t stdout_pipe;
    subprocess_pipe_t stderr_pipe;
} subprocess_pipe_def_t;


static void close_pipe(int *pipe) {
    close(pipe[0]);
    close(pipe[1]);
}

static int make_pipes(const subprocess_pipe_def_t* proc_def,
        int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2]) {
    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdin_pipe) {
        if (-1 == pipe(stdin_pipe)) {
            goto error;
        }
    }

    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdout_pipe) {
        if (-1 == pipe(stdout_pipe)) {
            goto error;
        }
    }

    if (SUBPROCESS_PIPE_NORMAL == proc_def->stderr_pipe) {
        if (-1 == pipe(stderr_pipe)) {
            goto error;
        }
    }

    return 0;
    error:
    close_pipe(stdin_pipe);
    close_pipe(stdout_pipe);
    close_pipe(stderr_pipe);
    return 1;
}

static void child_pipes(const subprocess_pipe_def_t* proc_def,
        int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2]) {
    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdin_pipe) {
        close(stdin_pipe[PIPE_WRITE]);
        dup2(stdin_pipe[PIPE_READ], FD_STDIN);
    }
    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdout_pipe) {
        close(stdout_pipe[PIPE_READ]);
        dup2(stdout_pipe[PIPE_WRITE], FD_STDOUT);
    }
    if (SUBPROCESS_PIPE_NORMAL == proc_def->stderr_pipe) {
        close(stderr_pipe[PIPE_READ]);
        // TODO: handle dup2 errors
        dup2(stderr_pipe[PIPE_WRITE], FD_STDERR);
    }
}

static void parent_pipes(const subprocess_pipe_def_t* proc_def, subprocess_run_t* proc_run,
        int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2]) {
    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdin_pipe) {
        close(stdin_pipe[PIPE_READ]);
        proc_run->stdin_fd = stdin_pipe[PIPE_WRITE];
    } else {
        proc_run->stdin_fd = -1;
    }

    if (SUBPROCESS_PIPE_NORMAL == proc_def->stdout_pipe) {
        close(stdout_pipe[PIPE_WRITE]);
        proc_run->stdout_fd = stdout_pipe[PIPE_READ];
    } else {
        proc_run->stdout_fd = -1;
    }

    if (SUBPROCESS_PIPE_NORMAL == proc_def->stderr_pipe) {
        close(stderr_pipe[PIPE_WRITE]);
        proc_run->stderr_fd = stderr_pipe[PIPE_READ];
    } else {
        proc_run->stderr_fd = -1;
    }
}

static int make_sharedmem(const subprocess_func_t* proc_def, void** mem_ptr, size_t* mem_size) {
    if (SUBPROCESS_SHAREDMEM_ANONYMOUS == proc_def->sharedmem) {
        void* mem = mmap(NULL, proc_def->sharedmem_size, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (NULL == mem) {
            return 1;
        }

        *mem_ptr = mem;
        *mem_size = proc_def->sharedmem_size;
    } else {
        *mem_ptr = NULL;
        *mem_size = 0;
    }

    return 0;
}

int subprocess_create(const subprocess_def_t* proc_def,
                      subprocess_run_t* proc_run) {
    memset(proc_run, 0, sizeof(subprocess_run_t));

    int result = 0;

    int stdin_pipe[2] = {0};
    int stdout_pipe[2] = {0};
    int stderr_pipe[2] = {0};

    if (make_pipes((subprocess_pipe_def_t*)proc_def,
                   stdin_pipe, stdout_pipe, stderr_pipe)) {
        result = SUBPROCESS_CREATE_ERROR_PIPE;
        goto error;
    }

    pid_t pid = fork();
    if (pid < 0) {
        result = SUBPROCESS_CREATE_ERROR_FORK;
        goto error;
    }
    if (0 == pid) {
        // child
        child_pipes((subprocess_pipe_def_t*)proc_def,
                    stdin_pipe, stdout_pipe, stderr_pipe);

        execve(proc_def->path, proc_def->argv, proc_def->envp);
        exit(errno);
    } else {
        // parent
        proc_run->pid = pid;

        parent_pipes((subprocess_pipe_def_t*)proc_def, proc_run,
                     stdin_pipe, stdout_pipe, stderr_pipe);
    }

    return 0;
error:
    close_pipe(stdin_pipe);
    close_pipe(stdout_pipe);
    close_pipe(stderr_pipe);

    return result;
}

int subprocess_create_func(const subprocess_func_t* proc_def,
                           subprocess_run_t* proc_run) {
    memset(proc_run, 0, sizeof(subprocess_run_t));

    int result = 0;

    int stdin_pipe[2] = {0};
    int stdout_pipe[2] = {0};
    int stderr_pipe[2] = {0};

    void* sharedmem = NULL;
    size_t sharedmem_size = 0;

    if (make_pipes((subprocess_pipe_def_t*)proc_def,
                   stdin_pipe, stdout_pipe, stderr_pipe)) {
        result = SUBPROCESS_CREATE_ERROR_PIPE;
        goto error;
    }
    if (make_sharedmem(proc_def, &sharedmem, &sharedmem_size)) {
        result = SUBPROCESS_CREATE_ERROR_SHAREDMEM;
        goto error;
    }

    pid_t pid = fork();
    if (pid < 0) {
        result = SUBPROCESS_CREATE_ERROR_FORK;
        goto error;
    }
    if (0 == pid) {
        // child
        child_pipes((subprocess_pipe_def_t*)proc_def,
                    stdin_pipe, stdout_pipe, stderr_pipe);

        subprocess_func_ctx_t context = {0};
        context.param = proc_def->param;
        context.param_size = proc_def->param_size;
        context.sharedmem = sharedmem;
        context.sharedmem_size = sharedmem_size;

        int exit_code = proc_def->entry_point(&context);
        exit(exit_code);
    } else {
        // parent
        proc_run->pid = pid;
        proc_run->sharedmem = sharedmem;
        proc_run->sharedmem_size = sharedmem_size;

        parent_pipes((subprocess_pipe_def_t*)proc_def, proc_run,
                     stdin_pipe, stdout_pipe, stderr_pipe);
    }

    return 0;
error:
    close_pipe(stdin_pipe);
    close_pipe(stdout_pipe);
    close_pipe(stderr_pipe);

    if (NULL != sharedmem) {
        munmap(sharedmem, sharedmem_size);
    }

    return result;
}

void subprocess_free(subprocess_run_t* proc_run) {
    subprocess_close_pipe(&proc_run->stdin_fd);
    subprocess_close_pipe(&proc_run->stdout_fd);
    subprocess_close_pipe(&proc_run->stderr_fd);

    if (NULL != proc_run->sharedmem) {
        munmap(proc_run->sharedmem, proc_run->sharedmem_size);
        proc_run->sharedmem = NULL;
        proc_run->sharedmem_size = 0;
    }
}

int subprocess_communicate(subprocess_run_t* proc_run,
        const void* input, size_t input_size,
        void* output, size_t output_size, size_t* output_read,
        void* error, size_t error_size, size_t* error_read) {
    int result = write(proc_run->stdin_fd, input, input_size);
    if (result < 0) {
        return errno;
    }
    subprocess_close_pipe(&proc_run->stdin_fd);

    result = read(proc_run->stdout_fd, output, output_size);
    if (result < 0) {
        return errno;
    }
    if (NULL != output_read) {
        *output_read = result;
    }
    subprocess_close_pipe(&proc_run->stdout_fd);

    result = read(proc_run->stderr_fd, error, error_size);
    if (result < 0) {
        return errno;
    }
    if (NULL != error_read) {
        *error_read = result;
    }
    subprocess_close_pipe(&proc_run->stderr_fd);

    return 0;
}

int subprocess_kill(const subprocess_run_t* proc_run, int signal) {
    if(kill(proc_run->pid, signal)) {
        return errno;
    }

    return 0;
}

int subprocess_wait(const subprocess_run_t* proc_run, int* exit_code) {
    int result = 0;
    int status;
    pid_t pid;

    while (1) {
        pid = waitpid(proc_run->pid, &status, 0);
        if (pid < 0) {
            if (ECHILD == errno) {
                result = SUBPROCESS_WAIT_NO_CHILD;
            } else {
                result = -errno;
            }

            *exit_code = 0;
            break;
        }
        if (WIFEXITED(status)) {
            *exit_code = WEXITSTATUS(status);
            result = SUBPROCESS_WAIT_EXIT_NORMAL;
            break;
        }
        if (WIFSIGNALED(status)) {
            *exit_code = WTERMSIG(status);
            result = SUBPROCESS_WAIT_EXIT_SIGNAL;
            break;
        }
    }

    return result;
}

void subprocess_close_pipe(int* fd_ptr) {
    if (*fd_ptr >= 0) {
        close(*fd_ptr);
        *fd_ptr = -1;
    }
}
