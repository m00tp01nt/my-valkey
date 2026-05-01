#pragma once

// perror()
#include <stdio.h>

typedef enum Response {

    RES_ERROR,
    RES_VALUE,
    RES_NOT_FOUND,
    RES_OK,
    RES_STATS,
    RES_BYE,

} Response;

typedef struct Result {

    Response response;

    char* message;

} Result;

char* generateResponseString(const Result result);

// OK to pass by value since we're only passing 
void freeResult(Result result);
