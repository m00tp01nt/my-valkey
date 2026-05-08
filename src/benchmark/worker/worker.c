#define _POSIX_C_SOURCE 200809L

#include "worker.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../common/command.h"
#include "../../common/kv.h"
#include "random.h"

#define BENCH_OPERATION_LENGTH 3
#define BENCH_STRING_LENGTH 3

#define READ_TRANSMISSION_LENGTH (BENCH_OPERATION_LENGTH + 1 + BENCH_STRING_LENGTH + 1)
#define WRITE_TRANSMISSION_LENGTH (BENCH_OPERATION_LENGTH + 1 + BENCH_STRING_LENGTH + 1 + BENCH_STRING_LENGTH + 1)
#define DELETE_TRANSMISSION_LENGTH (BENCH_OPERATION_LENGTH  + 1 + BENCH_STRING_LENGTH + 1)

/**
 * Instead of allocating a new string every request, 
 * allocate a single empty string, duplicate it once,
 * and memcpy arguments into it.
 * Very fragile, but (hopefully) very fast.
 */
#define READ_TEMPLATE "GET    \n"
#define READ_TEMPLATE_KEY_OFFSET 4

#define WRITE_TEMPLATE "PUT        \n"
#define WRITE_TEMPLATE_KEY_OFFSET 4
#define WRITE_TEMPLATE_VALUE_OFFSET 8

#define DELETE_TEMPLATE "DEL    \n"
#define DELETE_TEMPLATE_KEY_OFFSET 4

void* stress_test(void* args) {
    WorkerArguments* arguments = (WorkerArguments*) args;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(arguments->port);
    inet_pton(AF_INET, arguments->host, &addr.sin_addr);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return NULL;
    }

    char* get = strdup(READ_TEMPLATE);
    char* put = strdup(WRITE_TEMPLATE);
    char* del = strdup(DELETE_TEMPLATE);

    unsigned int fseed = arguments->seed;
    unsigned int normalizedReadPercent = (arguments->readPercent / 100.0f) * RAND_MAX;

    unsigned int randomValue;

    for (int i = 0; i < arguments->totalOperations; i++) {

        randomValue = rand_r(&fseed);

        switch ((int)(randomValue < normalizedReadPercent)) {
            case 1:
                memcpy(
                    get + READ_TEMPLATE_KEY_OFFSET,
                    &RANDOM_STRING[
                        /**
                         * Use the bottom 11 bits to index into the 
                         * long string in random.h, and grab the next 3 values.
                         * Our pregenerated string is 4095 chars long,
                         * so no out of bounds will happen.
                         */
                        randomValue & 0x7FF
                    ],
                    BENCH_STRING_LENGTH
                );
                if (write(fd, get, READ_TRANSMISSION_LENGTH) == -1) {
                    i = arguments->totalOperations;
                }
                break;
            
            // Delete written by Claude
            case 0:
                // 50-50 between PUT or DEL
                if (rand_r(&fseed) & 1) {
                    // memcpy random key
                    memcpy(
                        put + WRITE_TEMPLATE_KEY_OFFSET,
                        &RANDOM_STRING[
                            randomValue & 0x7FF
                        ],
                        BENCH_STRING_LENGTH
                    );
                    // memcpy randome value
                    memcpy(
                        put + WRITE_TEMPLATE_VALUE_OFFSET,
                        &RANDOM_STRING[
                            /**
                             * Bitshift right and grab the next 11 values
                             * to index into our random string for the value.
                             */
                            (randomValue >> 11) & 0x7FF
                        ],
                        BENCH_STRING_LENGTH
                    );
                    if (write(fd, put, WRITE_TRANSMISSION_LENGTH) == -1) {
                        i = arguments->totalOperations;
                    }
                } else {
                    memcpy(
                        del + DELETE_TEMPLATE_KEY_OFFSET,
                        &RANDOM_STRING[
                            randomValue & 0x7FF
                        ],
                        BENCH_STRING_LENGTH
                    );
                    if (write(fd, del, DELETE_TRANSMISSION_LENGTH) == -1) {
                        i = arguments->totalOperations;
                    }
                }
                break;

            default:
                break;
        }

        // Claude
        char c;
        while (read(fd, &c, 1) == 1 && c != '\n');
    }

    close(fd);

    // Then free our duped strings
    free(get);
    free(put);
    free(del);

    return NULL;
}