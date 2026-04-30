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

int hash(const char* key, const int bucketCount);

typedef struct HashtableEntry {

    ttl_t ttl;
    char* key;
    char* value;
    struct HashtableEntry* next;

} HashtableEntry;

typedef struct Hashtable {

    float loadFactor;
    int entries;
    int bucketCount;

    HashtableEntry** buckets;

} Hashtable;

Hashtable* hashtable_create(int bucketCount) {
    
    Hashtable* hashtable = malloc(sizeof(Hashtable));

    if (hashtable == NULL) {
        perror("Unable to allocate new Hashtable");
        return NULL;
    }

    HashtableEntry** buckets = (HashtableEntry**) calloc(bucketCount, sizeof(HashtableEntry));

    if (buckets == NULL) {
        perror("Unable to allocate buckets for new Hashtable");
        return NULL;
    }

    hashtable->entries = 0;
    hashtable->bucketCount = bucketCount;
    hashtable->buckets = buckets;

    return hashtable;
}

bool hashtable_set_load_factor(Hashtable* const hashtable, float loadFactor) {
    
    hashtable->loadFactor = loadFactor;

    return true;

}

bool hashtable_set(Hashtable* const hashtable, const char *key, const char *value) {
    
    int hashValue = hash(key, hashtable->bucketCount);

    HashtableEntry* bucketHead = hashtable->buckets[hashValue];

    HashtableEntry* entry = (HashtableEntry*) malloc(sizeof(HashtableEntry));

    char* tableKey = (char*) malloc((strlen(key) + 1) * sizeof(char));
    char* tableValue = (char*) malloc((strlen(value) + 1) * sizeof(char));

    strcpy(tableKey, key);
    strcpy(tableValue, value);

    if (bucketHead == NULL) {
        
        entry->key = tableKey;
        entry->value = tableValue;

        entry->next = NULL;

        bucketHead = entry;

        hashtable->entries++;

        return true;
    }

    return false;
}

bool hashtable_set_ttl(Hashtable *const hashtable, const char *key, const char *value, ttl_t ttl)
{
    return false;
}

char* hashtable_get(const Hashtable *hashtable, const char *key)
{
    return NULL;
}

bool hashtable_delete(const Hashtable *hashtable, const char *key)
{
    return false;
}

bool hashtable_destroy(Hashtable *hashtable) {

    free(hashtable->buckets);

    free(hashtable);

    return true;    
}

HashtableStatistics* hashtable_get_statistics(const Hashtable *hashtable) {
    return NULL;
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