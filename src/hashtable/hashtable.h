#pragma once

#include "../util/bool.h"
#include "../util/ttl.h"

// Types
typedef struct Hashtable Hashtable;

typedef struct {

    unsigned int entries;
    unsigned int misses;
    unsigned int hits;
    unsigned int deletes;
    unsigned int buckets;

} HashtableStatistics;

// Functions
Hashtable* hashtable_create(int bucketCount);

bool hashtable_put(Hashtable* hashtable, const char* key, const char* value);

bool hashtable_put_ttl(Hashtable* hashtable, const char* key, const char* value, ttl_t ttl);

char* hashtable_get(Hashtable* hashtable, const char* key);

bool hashtable_delete(Hashtable* hashtable, const char* key);

bool hashtable_destroy(Hashtable* hashtable);

HashtableStatistics hashtable_get_statistics(const Hashtable* hashtable);
