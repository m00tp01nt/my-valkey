#define _POSIX_C_SOURCE 200809L

#include "response.h"

#include <stdlib.h>
#include <string.h>

#include "../kv.h"

const char* responseToString(const Response response) {

    switch (response) {
        case RES_ERROR: return "ERROR";
        case RES_NOT_FOUND: return "NOT_FOUND";
        case RES_VALUE: return "VALUE";
        case RES_OK: return "OK";
        case RES_STATS: return "STATS";
        case RES_BYE: return "BYE";
    }

    return "ERROR";
}

char* generateResponseString(const Result result) {

    char* responseString;

    const char* responseType = responseToString(result.response);
    int responseTypeLength = strlen(responseType);

    if (result.message == NULL) {
        responseString = (char*) malloc((responseTypeLength + 2) * sizeof(char));
        strcpy(responseString, responseType);
        responseString[responseTypeLength] = RESPONSE_EPILOGUE_C;
        responseString[responseTypeLength + 1] = '\0';
        return responseString;
    }

    int bodyLength = strnlen(result.message, MAX_RESPONSE_BODY_LENGTH);

    responseString = (char*) malloc((responseTypeLength + 1 + bodyLength + 2) * sizeof(char));

    responseString[0] = '\0';
    strcat(responseString, responseType);
    strcat(responseString, " ");
    strcat(responseString, result.message);
    strcat(responseString, RESPONSE_EPILOGUE_S);
    
    return responseString;
}

void freeResult(Result result) {
    if (result.message != NULL)
        free(result.message);
}
