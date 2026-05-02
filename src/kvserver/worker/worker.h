#pragma once

#include "../hashtable/hashtable.h"
#include "../queue/queue.h"

typedef struct WorkerAguments {

    Queue* queue;
    Hashtable* hashtable;

} WorkerAguments;

void* kvserver_work(void* arg);

void handle_client(int connection, Hashtable* hashtable);