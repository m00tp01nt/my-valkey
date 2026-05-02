#define _GNU_SOURCE

#include "command.h"

#include <stdio.h>
#include <stdlib.h>

#include "kv.h"

char* commandToString(const Command* command) {
    
    char* buffer = (char*) malloc(MAX_LINE_LEN * sizeof(char));

    buffer[0] = '\0';

    strcat(buffer, operationToString(command->operation));
    
    if (command->key != NULL) {
        strcat(buffer, " ");
        strcat(buffer, command->key);
    }
    if (command->value != NULL) {
        strcat(buffer, " ");
        strcat(buffer, command->value);
    }
    if (command->ttl != 0) {
        char* ttlString;
        asprintf(&ttlString, " %u", command->ttl);
        strcat(buffer, ttlString);
    }

    strcat(buffer, "\n");

    char* asString = strdup(buffer);

    free(buffer);

    return asString;
}

void freeCommand(Command *command) {
    if (command->key != NULL) free(command->key);
    if (command->value != NULL) free(command->value);
    freeResult(command->result);
}
