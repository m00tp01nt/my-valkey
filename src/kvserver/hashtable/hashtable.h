#pragma once

#include <time.h>
#include <stdatomic.h>

#include "../../common/bool.h"
#include "../../common/ttl.h"

// Types
typedef struct Hashtable Hashtable;

typedef struct {

    atomic_uint_fast32_t entries;
    atomic_uint_fast32_t misses;
    atomic_uint_fast32_t puts;
    atomic_uint_fast32_t hits;
    atomic_uint_fast32_t deletes;
    atomic_uint_fast32_t buckets;

    time_t creationTime;

} HashtableStatistics;

// Functions
Hashtable* hashtable_create(int bucketCount);

bool hashtable_put(Hashtable* hashtable, const char* key, const char* value);

bool hashtable_put_ttl(Hashtable* hashtable, const char* key, const char* value, ttl_t ttl);

char* hashtable_get(Hashtable* hashtable, const char* key);

bool hashtable_delete(Hashtable* hashtable, const char* key);

bool hashtable_destroy(Hashtable* hashtable);

HashtableStatistics hashtable_get_statistics(const Hashtable* hashtable);

char* hashtable_get_statistics_string(const Hashtable* hashtable);