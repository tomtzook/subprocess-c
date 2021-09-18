#pragma once

#include <cstddef>
#include <cstdint>
#include <fcntl.h>

#include "except.h"
#include "pipes.h"
#include "sharedmem.h"

namespace subprocess {

enum class ExitType {
    NORMAL,
    SIGNAL
};

struct ProcessExit {
    ExitType type;
    int code;
};

enum Options {
    OPTION_PIPE_STDIN = (1 << 0),
    OPTION_PIPE_STDOUT = (1 << 1),
    OPTION_PIPE_STDERR = (1 << 2),
};

struct Context {
    void* param;
    size_t paramSize;
    void* sharedmem;
    size_t sharedmemSize;
};

using EntryPoint = int(*)(const Context&);

class Subprocess {
public:
    Subprocess(pid_t childPid, Pipe& stdin, Pipe& stdout, Pipe& stderr);
    Subprocess(Subprocess&& other) noexcept;
    virtual ~Subprocess();

    pid_t childPid() const;
    const Fd& childStdin() const;
    const Fd& childStdout() const;
    const Fd& childStderr() const;

    void signal(int signal);
    ProcessExit wait();

private:
    pid_t m_processPid;

    Pipe m_stdin;
    Pipe m_stdout;
    Pipe m_stderr;
};

class FunctionSubprocess : public Subprocess {
public:
    FunctionSubprocess(pid_t childPid, Pipe& stdin, Pipe& stdout, Pipe& stderr, SharedMemory& sharedMem);
    FunctionSubprocess(FunctionSubprocess&& other) noexcept;

    void* sharedMemory();

private:
    SharedMemory m_sharedMemory;
};

Subprocess open(char* path, char** argv, char** envp, int options);
FunctionSubprocess open(EntryPoint entryPoint, int options, void* param, size_t paramSize, size_t sharedMemSize = 0);

}
