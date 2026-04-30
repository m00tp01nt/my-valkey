#include "hashtable.h"

#include <string.h>

// malloc()
#include <stdlib.h>

// perror()
#include <stdio.h>

#include "../kv.h"

#include "../util/ttl.h"

#define HASHTABLE_INITIAL_SIZE 5
#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75

typedef struct HashtableEntry {

    ttl_t ttl;
    char* key;
    char* value;
    struct HashtableEntry* next;

} HashtableEntry;

typedef struct Hashtable {

    float loadFactor;
    int bucketCount;

    HashtableEntry** buckets;

    HashtableStatistics metadata;

} Hashtable;

int hash(const char* key, const int bucketCount);
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

    hashtable->metadata.entries = 0;
    hashtable->metadata.hits = 0;
    hashtable->metadata.misses = 0;
    hashtable->metadata.deletes = 0;
    hashtable->metadata.buckets = bucketCount;

    hashtable->buckets = buckets;

    return hashtable;
}

bool hashtable_set(Hashtable* hashtable, const char *key, const char *value) {
    return hashtable_set_ttl(hashtable, key, value, 0);
}

bool hashtable_set_ttl(Hashtable* hashtable, const char *key, const char *value, ttl_t ttl) {
    
    int hashValue = hash(key, hashtable->bucketCount);

    HashtableEntry* bucketHead = hashtable->buckets[hashValue];

    // First entry in bucket
    if (bucketHead == NULL) {

        HashtableEntry* entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));
        entry->key = strdup(key);
        entry->value = strdup(value);
        entry->ttl = ttl;
        entry->next = NULL;

        hashtable->buckets[hashValue] = entry;

        hashtable->metadata.entries++;

        return true;
    }

    // Check if key already in table
    HashtableEntry* entry = hashtable_get_entry(hashtable, key);
    if (entry != NULL) {
        free(entry->value);
        entry->value = strdup(value);
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
    hashtable->metadata.entries++;

    return true;
}

HashtableEntry* hashtable_get_entry(const Hashtable* hashtable, const char* key) {
    HashtableEntry* index = hashtable->buckets[hash(key, hashtable->bucketCount)];

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
    return entry->value;
}

bool hashtable_delete(Hashtable* hashtable, const char *key) {
    return false;
}

bool hashtable_destroy(Hashtable* hashtable) {


    for (int i = 0; i < hashtable->bucketCount; i++) {
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
