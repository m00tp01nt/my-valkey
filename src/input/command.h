#pragma once

// Operation type
#include "operation.h"

// ttl_t
#include "../util/ttl.h"

typedef struct Command {

    Operation operation;

    char* key;
    
    char* value;

    ttl_t ttl;

} Command;