#include "logger.h"

#include <stdio.h>

#include "../input/operation.h"

void logCommand(const Command *command) {

    printf("Command:\n");
    printf("\tOperation: %s\n", operationToString(command->operation));
    printf("\tKey: %s\n", command->key == NULL ? "\0" : command->key);
    printf("\tValue: %s\n", command->value == NULL ? "\0" : command->value);

    command->ttl == 0 ? printf("\tTTL: \n") : printf("\tTTL: %u\n", command->ttl);

}
