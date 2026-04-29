#pragma once

// perror()
#include <stdio.h>

typedef enum Response {

    NOT_FOUND,
    VALUE,
    OK,
    STATS,
    BYE,

} Response;

char* responseToString(const Response response) {

    switch (response) {

        case NOT_FOUND: return "NOT_FOUND";

        case VALUE: return "VALUE";
        
        case OK: return "OK";

        case STATS: return "STATS";

        case BYE: return "BYE";
        
        default: perror("Unknown response");
    }

}