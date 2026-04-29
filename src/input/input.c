// needed for strnlen
#define _POSIX_C_SOURCE 200809L

#include "input.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../util/bool.h"
#include "../kv.h"

#include "command.h"
#include "operation.h"
#include "parse.h"
#include "problem.h"

typedef struct Tokens {

    int tokenCount;

    char** tokens;

} Tokens;

bool hasCorrectTermination(const char* input);
Tokens tokenize(const char* input);
bool argumentsAreValid(Command* command);

Command parseInput(const char* input) {
    
    Command result = {0};

    result.problem = NULL;

    if (!hasCorrectTermination(input)) {
        result.problem = I_PROBLEM_BAD_TERMINATION;
        return result;
    }
    
    Tokens tokens = tokenize(input);

    if (tokens.tokenCount > MAX_TOKEN_COUNT) {
        result.problem = I_PROBLEM_TOO_MANY_ARGS;
        return result;
    }

    Operation operation = stringToOperation(tokens.tokens[0]);

    int arguments = tokens.tokenCount - 1;

    switch (operation) {

        case GET:
            if (arguments > O_GET_MAX_ARGS) {
                result.problem = I_PROBLEM_TOO_MANY_ARGS " | " O_GET_USAGE;
                break;
            }
            if (arguments < O_GET_MIN_ARGS) {
                result.problem = I_PROBLEM_TOO_FEW_ARGS " | " O_GET_USAGE;
                break;
            }
            
            result.operation = GET;
            result.key = tokens.tokens[1];
            break;

        case PUT:
            if (arguments > O_PUT_MAX_ARGS) {
                result.problem = I_PROBLEM_TOO_MANY_ARGS " | " O_PUT_USAGE;
                break;
            }
            if (arguments < O_PUT_MIN_ARGS) {
                result.problem = I_PROBLEM_TOO_FEW_ARGS " | " O_PUT_USAGE;
                break;
            }

            result.key = tokens.tokens[1];
            result.value = tokens.tokens[2];
            if (arguments == 3) result.ttl = (int) strtoul(tokens.tokens[3], NULL, 10);
            break;

        case DEL:
            if (arguments > O_DEL_MAX_ARGS) {
                result.problem = I_PROBLEM_TOO_MANY_ARGS " | " O_DEL_USAGE;
                break;
            }
            if (arguments < O_DEL_MIN_ARGS) {
                result.problem = I_PROBLEM_TOO_FEW_ARGS " | " O_DEL_USAGE;
                break;
            }

            result.key = tokens.tokens[1];

            break;

        case STATS:
            if (arguments > O_STATS_MAX_ARGS) {
                result.problem = I_PROBLEM_TOO_MANY_ARGS " | " O_STATS_USAGE;
                break;
            }
            if (arguments < O_STATS_MIN_ARGS) {
                result.problem = I_PROBLEM_TOO_FEW_ARGS " | " O_STATS_USAGE;
                break;
            }
            break;

        case QUIT:
            if (arguments > O_QUIT_MAX_ARGS) {
                result.problem = I_PROBLEM_TOO_MANY_ARGS " | " O_QUIT_USAGE;
                break;
            }
            if (arguments < O_QUIT_MIN_ARGS) {
                result.problem = I_PROBLEM_TOO_FEW_ARGS " | " O_QUIT_USAGE;
                break;
            }
            break;

        case UNKNOWN:
            result.problem = I_PROBLEM_UNKNOWN_OPERATION;
    }

    if (result.problem != NULL) return result;

    if (!argumentsAreValid(&result)) return result;

    return result;
}

bool hasCorrectTermination(const char* input) {

    char* lastTermination = strchr(input, I_TERMINATOR);

    if (lastTermination == NULL)
        return false;

    char* firstTermination = strchr(input, I_TERMINATOR);

    if (firstTermination == lastTermination)
        return true;

    return false;
}

Tokens tokenize(const char* input) {

    Tokens tokens = {0};

    tokens.tokenCount = 0;

    int delimiter[MAX_LINE_LEN >> 1];

    char item;

    // Grab the delimiter at the end of each token
    for (int i = 0; i < MAX_LINE_LEN; i++) {

        item = input[i];

        if (item == I_DELIMITER || item == I_TERMINATOR) {

            // Let multiple spaces count as one space
            if (i == 0 && item == I_DELIMITER) continue;
            if (i > 0 && input[i - 1] == I_DELIMITER) {}
            else
                delimiter[tokens.tokenCount++] = i;
        }

        if (item == I_TERMINATOR) break;
    }

    tokens.tokens = (char**) malloc(tokens.tokenCount * sizeof(char*));

    // Copy the tokens into new strings
    for (int i = 0; i < tokens.tokenCount; i++) {

        // Walk backwards until hitting the next delimiter
        int tokenLength = 0;
        int tokenStartIndex = delimiter[i];
        
        while (tokenStartIndex >= 0 && input[--tokenStartIndex] != I_DELIMITER) 
            tokenLength++;

        tokenStartIndex++;
        tokenLength--;

        char* token = (char*) malloc((tokenLength + 1) * sizeof(char));

        memcpy(token, input + tokenStartIndex, tokenLength);

        token[tokenLength] = '\0';

        tokens.tokens[i] = token;
    }

    return tokens;
}

bool argumentsAreValid(Command* command) {

    bool problem;

    problem = strnlen(command->key, MAX_KEY_LEN + 1) == MAX_KEY_LEN + 1 ? true : false;
    if (problem) {
        command->problem = I_PROBLEM_KEY_TOO_LONG;
        return false;
    }

    problem = strnlen(command->key, MAX_VAL_LEN + 1) == MAX_VAL_LEN + 1 ? true : false;
    if (problem) {
        command->problem = I_PROBLEM_VAL_TOO_LONG;
        return false;
    }

    problem = (command->ttl > MAX_TTL) ? true : false;
    if (problem) {
        command->problem = I_PROBLEM_TTL_TOO_LARGE;
        return false;
    }

    return true;
}