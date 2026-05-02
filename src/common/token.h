#pragma once

typedef struct Tokens {

    int tokenCount;
    char** tokens;

} Tokens;

Tokens tokenize(const char* input, char delimiter, char terminator);
void freeTokens(Tokens* tokens);