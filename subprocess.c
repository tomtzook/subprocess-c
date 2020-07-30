#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>

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
    if (PIPE == proc_def->stdin_pipe) {
        if (-1 == pipe(stdin_pipe)) {
            goto error;
        }
    }

    if (PIPE == proc_def->stdout_pipe) {
        if (-1 == pipe(stdout_pipe)) {
            goto error;
        }
    }

    if (PIPE == proc_def->stderr_pipe) {
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

static int child_pipes(const subprocess_pipe_def_t* proc_def,
        int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2]) {
    if (PIPE == proc_def->stdin_pipe) {
        close(stdin_pipe[PIPE_WRITE]);
        dup2(stdin_pipe[PIPE_READ], FD_STDIN);
    }
    if (PIPE == proc_def->stdout_pipe) {
        close(stdout_pipe[PIPE_READ]);
        dup2(stdout_pipe[PIPE_WRITE], FD_STDOUT);
    }
    if (PIPE == proc_def->stderr_pipe) {
        close(stderr_pipe[PIPE_READ]);
        // TODO: handle dup2 errors
        dup2(stderr_pipe[PIPE_WRITE], FD_STDERR);
    }

    return 0;
}

static void parent_pipes(const subprocess_pipe_def_t* proc_def, subprocess_run_t* proc_run,
        int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2]) {
    if (PIPE == proc_def->stdin_pipe) {
        close(stdin_pipe[PIPE_READ]);
        proc_run->stdin_fd = stdin_pipe[PIPE_WRITE];
    } else {
        proc_run->stdin_fd = -1;
    }

    if (PIPE == proc_def->stdout_pipe) {
        close(stdout_pipe[PIPE_WRITE]);
        proc_run->stdout_fd = stdout_pipe[PIPE_READ];
    } else {
        proc_run->stdout_fd = -1;
    }

    if (PIPE == proc_def->stderr_pipe) {
        close(stderr_pipe[PIPE_WRITE]);
        proc_run->stderr_fd = stderr_pipe[PIPE_READ];
    } else {
        proc_run->stderr_fd = -1;
    }
}



int subprocess_create(const subprocess_func_t* proc_def,
        subprocess_run_t* proc_run) {
    int stdin_pipe[2] = {0};
    int stdout_pipe[2] = {0};
    int stderr_pipe[2] = {0};
    if (make_pipes((subprocess_pipe_def_t*)proc_def,
            stdin_pipe, stdout_pipe, stderr_pipe)) {
        return 1;
    }


    pid_t pid = fork();
    if (pid < 0) {
        return 2;
    }
    if (0 == pid) {
        // child
        child_pipes((subprocess_pipe_def_t*)proc_def,
                stdin_pipe, stdout_pipe, stderr_pipe);

        int exit_code = proc_def->entry_point(proc_def->param, proc_def->param_size);
        exit(exit_code);
    } else {
        // parent
        proc_run->pid = pid;

        parent_pipes((subprocess_pipe_def_t*)proc_def, proc_run,
                stdin_pipe, stdout_pipe, stderr_pipe);
    }

    return 0;
}

int subprocess_create_shell(const subprocess_shell_t* proc_def,
                            subprocess_run_t* proc_run) {
    int stdin_pipe[2] = {0};
    int stdout_pipe[2] = {0};
    int stderr_pipe[2] = {0};
    if (make_pipes((subprocess_pipe_def_t*)proc_def,
                   stdin_pipe, stdout_pipe, stderr_pipe)) {
        return 1;
    }


    pid_t pid = fork();
    if (pid < 0) {
        return 2;
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
}

void subprocess_free(subprocess_run_t* proc_run) {
    if (proc_run->stdin_fd >= 0) {
        close(proc_run->stdin_fd);
    }
    if (proc_run->stdout_fd >= 0) {
        close(proc_run->stdout_fd);
    }
    if (proc_run->stderr_fd >= 0) {
        close(proc_run->stderr_fd);
    }
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
            *exit_code = WSTOPSIG(status);
            result = SUBPROCESS_WAIT_EXIT_SIGNAL;
            break;
        }
    }

    return result;
}

