// needed for strnlen
#define _POSIX_C_SOURCE 200809L

#include "input.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../../common/bool.h"
#include "../../common/logger.h"

#include "../../common/kv.h"

#include "../../common/command.h"
#include "../../common/operation.h"
#include "../../common/token.h"
#include "problem.h"

bool hasCorrectTermination(const char* input);
bool argumentsAreValid(Command* command);

Command parseInput(const char* input, char delimiter, char terminator) {
    
    Command result = {0};

    result.result.response = RES_OK;
    result.result.message = NULL;

    if (!hasCorrectTermination(input)) {
        result.result.response = RES_ERROR;
        result.result.message = strdup(I_PROBLEM_BAD_TERMINATION);
        return result;
    }
    
    Tokens tokens = tokenize(input, delimiter, terminator);

    if (tokens.tokenCount > MAX_TOKEN_COUNT) {
        result.result.response = RES_ERROR;
        result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS);
        return result;
    }

    Operation operation = stringToOperation(tokens.tokens[0]);
    result.operation = operation;

    int arguments = tokens.tokenCount - 1;

    switch (operation) {

        case GET:
            if (arguments > O_GET_MAX_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS " | " O_GET_USAGE);
                break;
            }
            if (arguments < O_GET_MIN_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_FEW_ARGS " | " O_GET_USAGE);
                break;
            }

            result.key = strdup(tokens.tokens[1]);

            break;

        case PUT:
            if (arguments > O_PUT_MAX_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS " | " O_PUT_USAGE);
                break;
            }
            if (arguments < O_PUT_MIN_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_FEW_ARGS " | " O_PUT_USAGE);
                break;
            }

            result.key = strdup(tokens.tokens[1]);
            result.value = strdup(tokens.tokens[2]);

            result.ttl = (arguments == 3) ? (ttl_t) strtoul(tokens.tokens[3], NULL, 10) : 0;
            break;

        case DEL:
            if (arguments > O_DEL_MAX_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS " | " O_DEL_USAGE);
                break;
            }
            if (arguments < O_DEL_MIN_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_FEW_ARGS " | " O_DEL_USAGE);
                break;
            }

            result.key = strdup(tokens.tokens[1]);
            break;

        case STATS:
            if (arguments > O_STATS_MAX_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS " | " O_STATS_USAGE);
                break;
            }
            if (arguments < O_STATS_MIN_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_FEW_ARGS " | " O_STATS_USAGE);
                break;
            }
            break;

        case QUIT:
            if (arguments > O_QUIT_MAX_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_MANY_ARGS " | " O_QUIT_USAGE);
                break;
            }
            if (arguments < O_QUIT_MIN_ARGS) {
                result.result.response = RES_ERROR;
                result.result.message = strdup(I_PROBLEM_TOO_FEW_ARGS " | " O_QUIT_USAGE);
                break;
            }
            break;

        case UNKNOWN:
            result.result.response = RES_ERROR;
            result.result.message = strdup(I_PROBLEM_UNKNOWN_OPERATION);
    }

    freeTokens(&tokens);

    return result;
}

char* readLine(int fd) {

    char* buffer = (char*) malloc((MAX_LINE_LEN + 1) * sizeof(char));

    // Adapted from Claude
    int i = 0;
    char c;
    while (i < MAX_LINE_LEN) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0){
            free(buffer);
            return NULL;
        }
        buffer[i++] = c;
        if (c == '\n') break;
    }

    if (i == MAX_LINE_LEN) {
        free(buffer);
        return NULL;
    }
    buffer[i] = '\0';

    char* line = strdup(buffer);
    free(buffer);
    return line;
}

bool hasCorrectTermination(const char* input) {

    char* lastTermination = strrchr(input, I_TERMINATOR);

    if (lastTermination == NULL)
        return false;

    char* firstTermination = strchr(input, I_TERMINATOR);

    if (firstTermination == lastTermination)
        return true;

    return false;
}

bool argumentsAreValid(Command* command) {

    if (
        command->operation == STATS || 
        command->operation == QUIT
    ) 
        return true;

    bool problem;

    problem = strnlen(command->key, MAX_KEY_LEN + 1) == MAX_KEY_LEN + 1 ? true : false;
    if (problem) {
        command->result.response = RES_ERROR;
        command->result.message = strdup(I_PROBLEM_KEY_TOO_LONG);
        return false;
    }

    if (
        command->operation == GET ||
        command->operation == DEL
    )
        return true;

    problem = strnlen(command->value, MAX_VAL_LEN + 1) == MAX_VAL_LEN + 1 ? true : false;
    if (problem) {
        command->result.response = RES_ERROR;
        command->result.message = strdup(I_PROBLEM_VAL_TOO_LONG);
        return false;
    }

    if (command->ttl == 0)
        return true;
    
    problem = (command->ttl > MAX_TTL) ? true : false;
    if (problem) {
        command->result.response = RES_ERROR;
        command->result.message = strdup(I_PROBLEM_TTL_TOO_LARGE);
        return false;
    }

    return true;
}