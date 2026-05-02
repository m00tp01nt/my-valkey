#define _GPU_SOURCE

#include "hashtable.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../common/kv.h"

#include "../../common/ttl.h"

#define HASHTABLE_INITIAL_SIZE 5
#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75

typedef struct HashtableEntry {

    ttl_t ttl;
    char* key;
    char* value;
    struct HashtableEntry* next;
    struct HashtableEntry* previous;

} HashtableEntry;

typedef struct Hashtable {

    float loadFactor;

    HashtableEntry** buckets;

    HashtableStatistics metadata;

} Hashtable;

int hash(const char* key, const int bucketCount);
HashtableEntry* hashtable_get_entry(const Hashtable* hashtable, const char* key);
void hashtable_free_entry(HashtableEntry* hashtableEntry);

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

    hashtable->metadata.entries = 0;
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
    
    int hashValue = hash(key, hashtable->metadata.buckets);

    HashtableEntry* bucketHead = hashtable->buckets[hashValue];

    // First entry in bucket
    if (bucketHead == NULL) {

        HashtableEntry* entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));
        entry->key = strdup(key);
        entry->value = strdup(value);
        entry->ttl = ttl;
        entry->next = NULL;
        entry->previous = NULL;

        hashtable->buckets[hashValue] = entry;

        hashtable->metadata.entries++;
        hashtable->metadata.puts++;

        return true;
    }

    // Check if key already in table
    HashtableEntry* entry = hashtable_get_entry((const Hashtable*) hashtable, key);
    if (entry != NULL) {
        free(entry->value);
        entry->value = strdup(value);
        hashtable->metadata.puts++;
        return true;
    }

    // Need to make a new entry
    entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));
    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->ttl = ttl;
    entry->next = NULL;

    HashtableEntry* index = bucketHead;
    while (index->next != NULL)
        index = index->next;

    index->next = entry;
    entry->previous = index;

    hashtable->metadata.entries++;
    hashtable->metadata.puts++;

    return true;
}

HashtableEntry* hashtable_get_entry(const Hashtable* hashtable, const char* key) {
    HashtableEntry* index = hashtable->buckets[hash(key, hashtable->metadata.buckets)];

    while (index != NULL) {
        if (!strcmp(index->key, key))
            return index;
        index = index->next;
    }

    return NULL;
}

char* hashtable_get(Hashtable* hashtable, const char *key) {

    HashtableEntry* entry = hashtable_get_entry(hashtable, key);

    if (entry == NULL) {
        hashtable->metadata.misses++;
        return NULL;
    }

    hashtable->metadata.hits++;
    return strdup(entry->value);
}

bool hashtable_delete(Hashtable* hashtable, const char *key) {
    
    HashtableEntry* entry = hashtable_get_entry(hashtable, key);

    // Entry isn't in the table
    if (entry == NULL) return false;

    // Entry is head
    if (entry->previous == NULL) {
        hashtable->buckets[hash(key, hashtable->metadata.buckets)] = entry->next;

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

    hashtable->metadata.entries--;
    hashtable->metadata.deletes++;
    
    hashtable_free_entry(entry);
    return true;
}

bool hashtable_destroy(Hashtable* hashtable) {

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

HashtableStatistics hashtable_get_statistics(const Hashtable *hashtable) {
    return hashtable->metadata;
}

char* hashtable_get_statistics_string(const Hashtable *hashtable) {
    HashtableStatistics stats = hashtable_get_statistics(hashtable);

    char* statsAsString;

    long long uptime = (time(NULL) - stats.creationTime);

    asprintf(
        &statsAsString,
        "keys=%d misses=%d puts=%d dels=%d active_conns=%d uptime_s=%lld",
            stats.entries,
            stats.misses,
            stats.puts,
            stats.deletes,
            -1,
            (long long) uptime
    );

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
