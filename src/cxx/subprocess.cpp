
#include <unistd.h>
#include <cerrno>
#include <wait.h>
#include <cstdlib>

#include "cxx/subprocess.h"


static constexpr int FD_STDIN = 0;
static constexpr int FD_STDOUT = 1;
static constexpr int FD_STDERR = 2;

#define OP_IS_PIPE_STDIN(op) (((op) & OPTION_PIPE_STDIN) != 0)
#define OP_IS_PIPE_STDOUT(op) (((op) & OPTION_PIPE_STDOUT) != 0)
#define OP_IS_PIPE_STDERR(op) (((op) & OPTION_PIPE_STDERR) != 0)

namespace subprocess {


Subprocess::Subprocess(pid_t childPid, Pipe& stdin, Pipe& stdout, Pipe& stderr)
    : m_processPid(childPid)
    , m_stdin(std::move(stdin))
    , m_stdout(std::move(stdout))
    , m_stderr(std::move(stderr))
{}

Subprocess::Subprocess(Subprocess&& other) noexcept
    : m_processPid(other.m_processPid)
    , m_stdin(std::move(other.m_stdin))
    , m_stdout(std::move(other.m_stdout))
    , m_stderr(std::move(other.m_stderr))
{}

Subprocess::~Subprocess() {

}

pid_t Subprocess::childPid() const {
    return m_processPid;
}

const Fd& Subprocess::childStdin() const {
    return m_stdin.writeFd();
}

const Fd& Subprocess::childStdout() const {
    return m_stdout.readFd();
}

const Fd& Subprocess::childStderr() const {
    return m_stderr.readFd();
}

void Subprocess::signal(int signal) {
    if(kill(m_processPid, signal)) {
        throw ErrnoException(errno);
    }
}

ProcessExit Subprocess::wait() {
    int status;
    pid_t pid;

    while (1) {
        pid = waitpid(m_processPid, &status, 0);
        if (pid < 0) {
            if (ECHILD == errno) {
                throw WaitNoChildException();
            } else {
                throw ErrnoException(-errno);
            }
        }

        if (WIFEXITED(status)) {
            return ProcessExit {ExitType::NORMAL, WEXITSTATUS(status)};
        }
        if (WIFSIGNALED(status)) {
            return ProcessExit {ExitType::SIGNAL, WTERMSIG(status)};
        }
    }
}

FunctionSubprocess::FunctionSubprocess(pid_t childPid, Pipe& stdin, Pipe& stdout, Pipe& stderr, SharedMemory& sharedMem)
    : Subprocess(childPid, stdin, stdout, stderr)
    , m_sharedMemory(std::move(sharedMem))
{}

FunctionSubprocess::FunctionSubprocess(FunctionSubprocess&& other) noexcept
    : Subprocess(std::move(other))
    , m_sharedMemory(std::move(other.m_sharedMemory))
{}

void* FunctionSubprocess::sharedMemory() {
    return m_sharedMemory.ptr();
}

static pid_t open(int options, Pipe& stdin, Pipe& stdout, Pipe& stderr) {
    if (OP_IS_PIPE_STDIN(options)) {
        stdin.open();
    }
    if (OP_IS_PIPE_STDOUT(options)) {
        stdout.open();
    }
    if (OP_IS_PIPE_STDERR(options)) {
        stderr.open();
    }

    pid_t pid = fork();
    if (pid < 0) {
        throw ErrnoException(errno);
    }

    if (0 == pid) {
        // child
        if (OP_IS_PIPE_STDIN(options)) {
            stdin.writeFd().reset();
            stdin.readFd().dup2(FD_STDIN);
        }
        if (OP_IS_PIPE_STDOUT(options)) {
            stdout.readFd().reset();
            stdout.writeFd().dup2(FD_STDOUT);
        }
        if (OP_IS_PIPE_STDERR(options)) {
            stderr.readFd().reset();
            stderr.writeFd().dup2(FD_STDERR);
        }
    } else {
        // parent
        if (OP_IS_PIPE_STDIN(options)) {
            stdin.readFd().reset();
        }
        if (OP_IS_PIPE_STDOUT(options)) {
            stdout.writeFd().reset();
        }
        if (OP_IS_PIPE_STDERR(options)) {
            stderr.writeFd().reset();
        }
    }

    return pid;
}

Subprocess open(const char* path, char* const* argv, char* const* envp, int options) {
    Pipe stdin;
    Pipe stdout;
    Pipe stderr;

    pid_t pid = open(options, stdin, stdout, stderr);
    if (0 == pid) {
        // child
        execve(path, argv, envp);
        exit(errno);
    }

    return Subprocess(pid, stdin, stdout, stderr);
}

FunctionSubprocess open(EntryPoint entryPoint, int options,
                        const void* param, size_t paramSize,
                        size_t sharedMemSize) {
    Pipe stdin;
    Pipe stdout;
    Pipe stderr;
    SharedMemory sharedMemory;

    if (0 < sharedMemSize) {
        sharedMemory.open(sharedMemSize);
    }

    pid_t pid = open(options, stdin, stdout, stderr);
    if (0 == pid) {
        // child
        Context context = {
                .param = param,
                .paramSize = paramSize,
                .sharedmem = sharedMemory.ptr(),
                .sharedmemSize = sharedMemory.size()
        };
        int result = entryPoint(context);
        exit(result);
    }

    return FunctionSubprocess(pid, stdin, stdout, stderr, sharedMemory);
}

}
