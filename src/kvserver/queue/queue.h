#pragma once

#include "../../common/bool.h"

typedef struct QueueEntry {

    int fd;

} QueueEntry;

typedef struct Queue Queue;

Queue* queue_create(int capcity);

QueueEntry* queue_pop(Queue* queue);
bool queue_add(Queue* queue, QueueEntry entry);
bool queue_drain(Queue* queue);

void queue_destroy(Queue* queue);
void queue_entry_destroy(QueueEntry* entry);