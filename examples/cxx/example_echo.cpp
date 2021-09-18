#include <cstdio>

#include "cxx/subprocess.h"

int main(int argc, char** argv) {
    char buffer[64];

    char* const argv_[] = {"", "hello world", nullptr};
    char* const envp_[] = {nullptr};

    subprocess::Subprocess subprocess = subprocess::open("/bin/echo", argv_, envp_,
                                                         subprocess::OPTION_PIPE_STDOUT |
                                                         subprocess::OPTION_PIPE_STDERR);
    printf("child process %d\n", subprocess.childPid());

    subprocess::ProcessExit result = subprocess.wait();
    switch (result.type) {
        case subprocess::ExitType::NORMAL:
            printf("Subprocess exited normally: code=0x%x\n", result.code);
            break;
        case subprocess::ExitType::SIGNAL:
            printf("Subprocess exited with signal: signal=0x%x\n", result.code);
            break;
    }

    size_t readCount = subprocess.childStdout().read(buffer, 63);
    buffer[readCount] = '\0';
    printf("stdout: %s", buffer);

    readCount = subprocess.childStderr().read(buffer, 63);
    buffer[readCount] = '\0';
    printf("stderr: %s", buffer);

    return 0;
}

