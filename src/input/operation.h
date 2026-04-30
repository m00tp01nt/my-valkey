#pragma once

// MAX_
#include "../kv.h"

// strncmp()
#include <string.h>

typedef enum Operation {

    GET,
    PUT,
    DEL,
    STATS,
    QUIT,

    UNKNOWN,

} Operation;

Operation stringToOperation(const char* input);

const char* operationToString(Operation operation);