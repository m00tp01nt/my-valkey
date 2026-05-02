#include "token.h"

#include <string.h>
#include <stdlib.h>

#include "kv.h"

Tokens tokenize(const char* input, char delimiterChar, char terminatorChar) {

    Tokens tokens = {0};

    tokens.tokenCount = 0;

    int delimiter[MAX_LINE_LEN >> 1];

    char item;

    // Grab the delimiter at the end of each token
    for (int i = 0; i < MAX_LINE_LEN; i++) {

        item = input[i];

        if (item == delimiterChar || item == terminatorChar) {

            // Let multiple spaces count as one space
            if (i == 0 && item == delimiterChar) continue;
            if (i > 0 && input[i - 1] == delimiterChar) {}
            else
                delimiter[tokens.tokenCount++] = i;
        }

        if (item == terminatorChar) break;
    }

    tokens.tokens = (char**) malloc(tokens.tokenCount * sizeof(char*));

    // Copy the tokens into new strings
    for (int i = 0; i < tokens.tokenCount; i++) {

        // Walk backwards until hitting the next delimiter
        int tokenLength = 0;
        int tokenStartIndex = delimiter[i] - 1;
        
        while (tokenStartIndex >= 0 && input[tokenStartIndex] != delimiterChar) {
            tokenLength++;
            tokenStartIndex--;
        }
        tokenStartIndex++;

        char* token = (char*) malloc((tokenLength + 1) * sizeof(char));

        memcpy(token, input + tokenStartIndex, tokenLength);

        token[tokenLength] = '\0';

        tokens.tokens[i] = token;
    }

    return tokens;
}

void freeTokens(Tokens* tokens) {
    for (int i = 0; i < tokens->tokenCount; i++)
        free(tokens->tokens[i]);
    
    free(tokens->tokens);
}
