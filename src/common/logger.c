#include "logger.h"

#include <stdio.h>

#include "operation.h"

void logCommand(const Command *command) {

    printf("Command:\n");
    printf("\t\tOperation: %s\n", operationToString(command->operation));
    printf("\t\tKey: %s\n", command->key == NULL ? "\0" : command->key);
    printf("\t\tValue: %s\n", command->value == NULL ? "\0" : command->value);
    command->ttl == 0 ? printf("\t\tTTL: \n") : printf("\t\tTTL: %u\n", command->ttl);

}
