#define _POSIX_C_SOURCE 200809L

#include "worker.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../common/command.h"
#include "../../common/kv.h"
#include "random.h"

char* generateString(unsigned int*, int);
Command* generateRead(unsigned int*);
Command* generateWrite(unsigned int*);

void* stress_test(void* args) {
    WorkerArguments* arguments = (WorkerArguments*) args;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(arguments->port);
    inet_pton(AF_INET, arguments->host, &addr.sin_addr);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return NULL;
    }

    char response[MAX_LINE_LEN];
    Command* command;
    char* commandAsString;

    for (int i = 0; i < arguments->totalOperations; i++) {

        command = (rand_r(&arguments->seed) % 100 < arguments->readPercent)
                    ? generateRead(&arguments->seed)
                    : generateWrite(&arguments->seed);
        
        commandAsString = commandToString(command);
        freeCommand(command);

        write(fd, commandAsString, strlen(commandAsString));
        free(commandAsString);

        read(fd, response, MAX_LINE_LEN);
    }

    return NULL;
}

Command* generateWrite(unsigned int* seed) {

    Command* command = (Command*) malloc(sizeof(Command));

    command->operation = PUT;
    command->key = generateString(seed, 3);
    command->value = generateString(seed, 3);

    return command;
}

Command* generateRead(unsigned int* seed) {

    Command* command = (Command*) malloc(sizeof(Command));

    command->operation = GET;
    command->key = generateString(seed, 3);

    return command;
}

char* generateString(unsigned int* seed, int length) {

    if (length > RANDOM_STRING_LENGTH)
        return NULL;
    
    return strndup((const char*)&RANDOM_STRING[rand_r(seed) % (RANDOM_STRING_LENGTH - length)], length);
}