#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "cxx/subprocess.h"


int in_new_proc(const subprocess::Context& ctx) {
    strcpy(reinterpret_cast<char*>(ctx.sharedmem), "hello world!");

    return 0;
}

#define MEM_SIZE (30)

int main(int argc, char** argv) {
    char buffer[MEM_SIZE];

    subprocess::FunctionSubprocess subprocess = subprocess::open(in_new_proc, 0,
                                                                 nullptr, 0,
                                                                 MEM_SIZE);
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

    strcpy(buffer, reinterpret_cast<char*>(subprocess.sharedMemory()));
    printf("From child via memory: %s\n", buffer);

    return 0;
}



