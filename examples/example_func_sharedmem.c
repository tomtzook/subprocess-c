#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "subprocess.h"


int in_new_proc(const subprocess_func_ctx_t* ctx) {
    strcpy(ctx->sharedmem, "hello world!");

    return 0;
}

#define MEM_SIZE (30)

int main(int argc, char** argv) {
    char buffer[MEM_SIZE];

    subprocess_func_t def = {
            .entry_point = in_new_proc,
            .options = SUBPROCESS_OPTION_SHAREDMEM_ANON,
            .sharedmem_size = MEM_SIZE
    };

    subprocess_run_t proc;

    int result = subprocess_create_func(&def, &proc);
    if (result) {
        printf("Error starting subprocess %d\n", result);
        return 1;
    }
    printf("child process %d\n", proc.pid);

    int exit_code;
    result = subprocess_wait(&proc, &exit_code);
    if (result < 0) {
        printf("Error waiting for subprocess %d\n", result);
    } else {
        printf("Done waiting for subprocess %d, exit code %d\n", result, exit_code);
    }

    strcpy(buffer, proc.sharedmem);
    printf("From child via memory: %s\n", buffer);

    subprocess_free(&proc);
    return 0;
}



