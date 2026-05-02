#pragma once

#include "../../common/command.h"

typedef struct WorkerArguments {

    const char* host;
    int port;

    unsigned int seed;
    long totalOperations;
    int readPercent;

} WorkerArguments;

void* stress_test(void* args);