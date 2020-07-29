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


int subprocess_create(const subprocess_def_t* proc_def,
        subprocess_run_t* proc_run) {

    int stdin_pipe[2];
    if (PIPE == proc_def->stdin_pipe) {
        if (-1 == pipe(stdin_pipe)) {
            return 1;
        }
    }

    int stdout_pipe[2];
    if (PIPE == proc_def->stdout_pipe) {
        if (-1 == pipe(stdout_pipe)) {
            return 1;
        }
    }

    int stderr_pipe[2];
    if (PIPE == proc_def->stderr_pipe) {
        if (-1 == pipe(stderr_pipe)) {
            return 1;
        }
    }


    pid_t pid = fork();
    if (pid < 0) {
        return 2;
    }
    if (0 == pid) {
        // child
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

        int exit_code = proc_def->entry_point(proc_def->param, proc_def->param_size);
        exit(exit_code);
    } else {
        // parent
        proc_run->pid = pid;

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
                result = 2;
            } else {
                result = -1;
            }

            break;
        }
        if (WIFEXITED(status)) {
            *exit_code = WEXITSTATUS(status);
            result = 0;
            break;
        }
        if (WIFSIGNALED(status)) {
            *exit_code = WSTOPSIG(status);
            result = 1;
            break;
        }
    }

    return result;
}

