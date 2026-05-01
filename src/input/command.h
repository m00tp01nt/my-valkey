#pragma once

#include "operation.h"
#include "../util/ttl.h"
#include "../response/response.h"

typedef struct Command {

    Operation operation;

    char* key;
    
    char* value;

    ttl_t ttl;

    // const char* problem;

    Result result;

} Command;