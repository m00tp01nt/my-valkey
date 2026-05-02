#include "queue.h"

#include <stdlib.h>
#include <pthread.h>

#include "../../common/bool.h"

typedef struct Queue {

    bool draining;
    int capacity;
    int size;
    int headIndex;
    int tailIndex;
    QueueEntry* entries;

    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

} Queue;


Queue* queue_create(int capacity) {
    
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queue->size = 0;
    queue->headIndex = 0;
    queue->tailIndex = 0;
    queue->draining = false;
    queue->capacity = capacity;
    queue->entries = (QueueEntry*) malloc(capacity * sizeof(QueueEntry));

    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);

    return queue;
}

QueueEntry* queue_pop(Queue *queue) {
    pthread_mutex_lock(&queue->lock);

    while (queue->size == 0 && !queue->draining)
        pthread_cond_wait(&queue->not_empty, &queue->lock);

    if (queue->size == 0 && queue->draining) {
        pthread_cond_broadcast(&queue->not_empty);
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    QueueEntry* value = (QueueEntry*) malloc(sizeof(QueueEntry));
    value->fd = queue->entries[queue->headIndex].fd;
    queue->headIndex = (queue->headIndex + 1) % queue->capacity;
    queue->size--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->lock);

    return value;
}

bool queue_add(Queue *queue, QueueEntry entry) {

    pthread_mutex_lock(&queue->lock);

    while (queue->size == queue->capacity && !queue->draining)
        pthread_cond_wait(&queue->not_full, &queue->lock);

    if (queue->draining == true){
        pthread_mutex_unlock(&queue->lock);
        return false;
    }

    queue->entries[queue->tailIndex].fd = entry.fd;

    queue->tailIndex = (queue->tailIndex + 1) % queue->capacity;
    queue->size++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);

    return true;
}

bool queue_drain(Queue *queue) {

    pthread_mutex_lock(&queue->lock);

    queue->draining = true;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);

    pthread_mutex_unlock(&queue->lock);
    return true;
}

void queue_destroy(Queue* queue) {

    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);

    free(queue->entries);
    free(queue);
}

void queue_entry_destroy(QueueEntry* entry) {
    free(entry);
}