#define _GNU_SOURCE

#include "hashtable.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../../common/kv.h"

#include "../../common/ttl.h"

#define HASHTABLE_INITIAL_SIZE 5
#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75

typedef struct HashtableMetadata {

    atomic_uint entries;
    atomic_uint misses;
    atomic_uint puts;
    atomic_uint hits;
    atomic_uint deletes;

    // const
    unsigned int buckets;
    time_t creationTime;

} HashtableMetadata;

typedef struct HashtableEntry {

    time_t expiration;
    char* key;
    char* value;
    struct HashtableEntry* next;
    struct HashtableEntry* previous;

} HashtableEntry;

typedef struct Hashtable {

    float loadFactor;
    HashtableEntry** buckets;
    HashtableMetadata metadata;

    pthread_rwlock_t* lock;

} Hashtable;

int hash(const char* key, const int bucketCount);
HashtableEntry* hashtable_get_entry(Hashtable* hashtable, const char* key);
void hashtable_free_entry(HashtableEntry* hashtableEntry);
bool hashtable_remove_entry(Hashtable*, HashtableEntry*, int bucket);

void hashtable_job_janitor(Hashtable* hashtable) {

    time_t now = time(NULL);

    for (unsigned int i = 0; i < hashtable->metadata.buckets; i++) {
        pthread_rwlock_wrlock(&hashtable->lock[i]);

        HashtableEntry* entry = hashtable->buckets[i];
        HashtableEntry* next;

        while (entry != NULL) {
            next = entry->next;
            if (entry->expiration != 0 && entry->expiration < now)
                hashtable_remove_entry(hashtable, entry, i);
            entry = next;
        }

        pthread_rwlock_unlock(&hashtable->lock[i]);
    }
    
}

Hashtable* hashtable_create(int bucketCount) {

    Hashtable* hashtable = malloc(sizeof(Hashtable));

    if (hashtable == NULL) {
        perror("Unable to allocate new Hashtable");
        return NULL;
    }

    HashtableEntry** buckets = (HashtableEntry**) calloc(bucketCount, sizeof(HashtableEntry*));

    if (buckets == NULL) {
        perror("Unable to allocate buckets for new Hashtable");
        return NULL;
    }

    hashtable->buckets = buckets;

    hashtable->lock = (pthread_rwlock_t*) malloc(bucketCount * sizeof(pthread_rwlock_t));
    for (int i = 0; i < bucketCount; i++)
        pthread_rwlock_init((hashtable->lock + i), NULL);

    hashtable->metadata.entries = 0;
    hashtable->metadata.puts = 0;
    hashtable->metadata.hits = 0;
    hashtable->metadata.misses = 0;
    hashtable->metadata.deletes = 0;
    hashtable->metadata.buckets = bucketCount;
    hashtable->metadata.creationTime = time(NULL);

    return hashtable;
}

bool hashtable_put(Hashtable* hashtable, const char *key, const char *value) {
    return hashtable_put_ttl(hashtable, key, value, 0);
}

bool hashtable_put_ttl(Hashtable* hashtable, const char *key, const char *value, ttl_t ttl) {
    
    int bucket = hash(key, hashtable->metadata.buckets);
    
    pthread_rwlock_wrlock(&hashtable->lock[bucket]);

    HashtableEntry* bucketHead = hashtable->buckets[bucket];

    // First entry in bucket
    if (bucketHead == NULL) {

        HashtableEntry* entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));
        entry->key = strdup(key);
        entry->value = strdup(value);
        entry->expiration = ttl == 0 ? 0 : time(NULL) + ttl;
        entry->next = NULL;
        entry->previous = NULL;

        hashtable->buckets[bucket] = entry;

        hashtable->metadata.entries++;
        hashtable->metadata.puts++;

        pthread_rwlock_unlock(&hashtable->lock[bucket]);
        return true;
    }

    // Check if key already in table
    HashtableEntry* entry = hashtable_get_entry(hashtable, key);
    if (entry != NULL) {
        free(entry->value);
        entry->value = strdup(value);
        entry->expiration = ttl == 0 ? 0 : time(NULL) + ttl;
        hashtable->metadata.puts++;

        pthread_rwlock_unlock(&hashtable->lock[bucket]);
        return true;
    }

    // Need to make a new entry
    entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));
    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->expiration = ttl == 0 ? 0 : time(NULL) + ttl;
    entry->next = NULL;

    HashtableEntry* index = bucketHead;
    while (index->next != NULL)
        index = index->next;

    index->next = entry;
    entry->previous = index;

    hashtable->metadata.entries++;
    hashtable->metadata.puts++;

    pthread_rwlock_unlock(&hashtable->lock[bucket]);
    return true;
}

// Caller must have read lock to call!
HashtableEntry* hashtable_get_entry(Hashtable* hashtable, const char* key) {
    HashtableEntry* index = hashtable->buckets[hash(key, hashtable->metadata.buckets)];
    while (index != NULL) {
        if (!strcmp(index->key, key)) {
            if (index->expiration != 0 && index->expiration < time(NULL)) 
                return NULL;
            else
                return index;
        }
        index = index->next;
    }
    return NULL;
}

char* hashtable_get(Hashtable* hashtable, const char *key) {

    int bucket = hash(key, hashtable->metadata.buckets);

    pthread_rwlock_rdlock(&hashtable->lock[bucket]);

    HashtableEntry* entry = hashtable_get_entry(hashtable, key);

    if (entry == NULL) {
        hashtable->metadata.misses++;
        pthread_rwlock_unlock(&hashtable->lock[bucket]);
        return NULL;
    }

    char* value = strdup(entry->value);

    hashtable->metadata.hits++;

    pthread_rwlock_unlock(&hashtable->lock[bucket]);
    return value;
}

bool hashtable_remove_entry(Hashtable* hashtable, HashtableEntry* entry, int bucket) {
    // Entry isn't in the table
    if (entry == NULL) return false;

    // Entry is head
    if (entry->previous == NULL) {
        hashtable->buckets[bucket] = entry->next;

        // Promote second entry to head
        if (entry->next != NULL) entry->next->previous = NULL;
    }
    // Entry is tail
    else if (entry->next == NULL) {
        entry->previous->next = NULL;
    }
    else {
        entry->previous->next = entry->next;
        entry->next->previous = entry->previous;   
    }

    hashtable_free_entry(entry);

    hashtable->metadata.entries--;
    hashtable->metadata.deletes++;
    return true;
}

bool hashtable_delete(Hashtable* hashtable, const char *key) {

    int bucket = hash(key, hashtable->metadata.buckets);

    pthread_rwlock_wrlock(&hashtable->lock[bucket]);
    
    HashtableEntry* entry = hashtable_get_entry(hashtable, key);

    // Entry isn't in the table
    if (entry == NULL) {
        pthread_rwlock_unlock(&hashtable->lock[bucket]);
        return false;
    }

    hashtable_remove_entry(hashtable, entry, bucket);

    pthread_rwlock_unlock(&hashtable->lock[bucket]);
    return true;
}

HashtableStatistics hashtable_get_statistics(Hashtable *hashtable) {

    HashtableStatistics stats = {
        .deletes        = atomic_load(&(hashtable->metadata.deletes)),
        .entries        = atomic_load(&(hashtable->metadata.entries)),
        .hits           = atomic_load(&(hashtable->metadata.hits)),
        .misses         = atomic_load(&(hashtable->metadata.misses)),
        .puts           = atomic_load(&(hashtable->metadata.puts)),
        
        .buckets        = hashtable->metadata.buckets,
        .creationTime   = hashtable->metadata.creationTime,
    };
    
    return stats;
}

bool hashtable_destroy(Hashtable* hashtable) {

    for (unsigned int i = 0 ; i < hashtable->metadata.buckets; i++) {
        pthread_rwlock_destroy(&hashtable->lock[i]);
    }
    free(hashtable->lock);

    for (unsigned int i = 0; i < hashtable->metadata.buckets; i++) {
        HashtableEntry* head = hashtable->buckets[i];

        if (hashtable->buckets[i] == NULL) continue;

        HashtableEntry* index;
        while (head->next != NULL) {
            index = head->next;
            head->next = index->next;

            hashtable_free_entry(index);
        }
        hashtable_free_entry(head);
    }

    free(hashtable->buckets);
    free(hashtable);

    return true;
}

char* hashtable_get_statistics_string(Hashtable *hashtable, unsigned int connections) {
    HashtableStatistics stats = hashtable_get_statistics(hashtable);

    char* statsAsString;

    long long uptime = (time(NULL) - stats.creationTime);

    if (asprintf(
        &statsAsString,
        "keys=%d hits=%d misses=%d puts=%d dels=%d active_conns=%u uptime_s=%lld",
            stats.entries,
            stats.hits,
            stats.misses,
            stats.puts,
            stats.deletes,
            connections,
            (long long) uptime
    ) < 0)
        return NULL;

    return statsAsString;
}

// djb2 hash function
int hash(const char* key, const int bucketCount) {

    unsigned long hash = 5381;

    for (size_t i = 0; i < MAX_KEY_LEN; i++) {

        char c = key[i];

        if (c == '\0') break;

        hash = ((hash << 5) + hash) + c;
    }

    return (int) (hash % bucketCount);
}

void hashtable_free_entry(HashtableEntry* hashtableEntry) {
    free(hashtableEntry->key);
    free(hashtableEntry->value);
    free(hashtableEntry);
}
