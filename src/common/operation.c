#include "operation.h"

#include "kv.h"

#define OP_GET_S "GET"
#define OP_PUT_S "PUT"
#define OP_DEL_S "DEL"
#define OP_STATS_S "STATS"
#define OP_QUIT_S "QUIT"

Operation stringToOperation(const char* input) {

    if (!strncmp(input, OP_GET_S, MAX_OPERATION_LEN + 1))   return GET;
    if (!strncmp(input, OP_PUT_S, MAX_OPERATION_LEN + 1))   return PUT;
    if (!strncmp(input, OP_DEL_S, MAX_OPERATION_LEN + 1))   return DEL;
    if (!strncmp(input, OP_STATS_S, MAX_OPERATION_LEN + 1)) return STATS;
    if (!strncmp(input, OP_QUIT_S, MAX_OPERATION_LEN + 1))  return QUIT;
    
    return UNKNOWN;
}

const char* operationToString(Operation operation) {

    switch (operation)
    {
        case GET: return "GET";
        case PUT: return "PUT";
        case DEL: return "DEL";
        case STATS: return "STATS";
        case QUIT: return "QUIT";
        case UNKNOWN: return "UNKNOWN";
    }
    
    return "UNDEFINED";
}
