#pragma once

#include "operation.h"
#include "ttl.h"
#include "response.h"

typedef struct Command {

    Operation operation;

    char* key;
    
    char* value;

    ttl_t ttl;

    Result result;

} Command;

char* commandToString(const Command* command);
void freeCommand(Command* command);