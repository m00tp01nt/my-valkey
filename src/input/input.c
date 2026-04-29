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

bool hasCorrectTermination(const char* input);

typedef struct Tokens {

    bool success;

    int tokenCount;

    char** tokens;

} Tokens;

Command parseInput(const char* input) {
    
    Command result = {0};

    result.problem = NULL;

    if (!hasCorrectTermination(input)) {
        result.problem = I_PROBLEM_BAD_TERMINATION;
        return result;
    }

    

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

    tokens.success = false;
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

        char* token = (char*) malloc((tokenLength + 1) * sizeof(char));

        memcpy(token, input + tokenStartIndex, tokenLength);

        token[tokenLength] = '\0';

        tokens.tokens[i] = token;
    }

    return tokens;
}

int main(void) {

    Tokens tokens = tokenize("Hello this is a test\n");

    for (size_t i = 0; i < tokens.tokenCount; i++) {
        printf("%s\n", tokens.tokens[i]);
    }
    

}