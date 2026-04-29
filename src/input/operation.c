#include "operation.h"

#define GET_S "GET"
#define PUT_S "PUT"
#define DEL_S "DEL"
#define STATS_S "STATS"
#define QUIT_S "QUIT"

Operation stringToOperation(const char* input) {

    if (!strncmp(input, GET_S, MAX_OPERATION_LEN))      return GET;
    if (!strncmp(input, PUT_S, MAX_OPERATION_LEN))      return PUT;
    if (!strncmp(input, DEL_S, MAX_OPERATION_LEN))      return DEL;
    if (!strncmp(input, STATS_S, MAX_OPERATION_LEN))    return STATS;
    if (!strncmp(input, QUIT_S, MAX_OPERATION_LEN))    return STATS;
    
    return UNKNOWN;
}
