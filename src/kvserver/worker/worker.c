#include "worker.h"

#include <stdlib.h>
#include <unistd.h>

#include "../input/input.h"
#include "../../common/kv.h"
#include "../../common/logger.h"

void* kvserver_work(void* arg) {

    WorkerAguments* args = (WorkerAguments*) arg;

    while (1) {
        QueueEntry* entry = queue_pop(args->queue);
        if (entry == NULL) {
            queue_entry_destroy(entry);
            break;
        }
        handle_client(entry->fd, args->hashtable);
        queue_entry_destroy(entry);
    }

    return NULL;
}

void handle_client(int connection, Hashtable *hashtable)
{

    char* buffer;

    while ((buffer = readLine(connection)) != NULL) {
        Command command = parseInput(buffer, I_DELIMITER, I_TERMINATOR);

        logCommand(&command);

        if (command.result.response == RES_ERROR) {
            char* response = generateResponseString(command.result);
            write(connection, response, strlen(response));
            free(response);
            freeCommand(&command);
            continue;
        }

        switch (command.operation) {
            case GET:
                {
                    char* value = hashtable_get(hashtable, command.key);
                    if (value == NULL) {
                        command.result.response = RES_NOT_FOUND;
                        command.result.message = NULL;
                    }
                    else {
                        command.result.response = RES_VALUE;
                        command.result.message = strdup(value);
                    }
                    free(value);
                }
                break;
            
            case PUT:
                {
                    bool result = hashtable_put(hashtable, command.key, command.value);
                    if (result == false) {
                        command.result.response = RES_ERROR;
                    }
                    else {
                        command.result.response = RES_OK;
                    }
                    command.result.message = NULL;
                }
                break;

            case DEL:
                {
                    bool result = hashtable_delete(hashtable, command.key);
                    if (result == false) {
                        command.result.response = RES_NOT_FOUND;
                    }
                    else {
                        command.result.response = RES_OK;
                    }
                    command.result.message = NULL;
                }
                break;

            case STATS:
                {
                    char* stats = hashtable_get_statistics_string(hashtable);
                    if (stats == NULL) {
                        command.result.response = RES_ERROR;
                        command.result.message = NULL;
                    }
                    else {
                        command.result.response = RES_STATS;
                        command.result.message = strdup(stats);
                    }
                    free(stats);
                }
                break;

            case QUIT:
                freeCommand(&command);
                close(connection);
                free(buffer);
                return;

            case UNKNOWN:
                perror("Unimplemented");
        }

        char* response = generateResponseString(command.result);
        write(connection, response, strlen(response));
        free(response);
        freeCommand(&command);
    }

    close(connection);
    free(buffer);
}