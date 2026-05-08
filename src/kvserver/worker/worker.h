#pragma once

#include <stdatomic.h>

#include "../hashtable/hashtable.h"
#include "../queue/queue.h"

typedef struct WorkerAguments {

    atomic_uint* connections;

    Queue* queue;
    Hashtable* hashtable;

} WorkerAguments;

void* kvserver_work(void* arg);
