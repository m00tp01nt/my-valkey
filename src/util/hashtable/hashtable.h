#pragma once

#include "../../util/bool.h"

// Types
typedef struct Hashtable Hashtable;

typedef struct {

    int entries;

} HashtableStatistics;

// Functions
const Hashtable* hashtable_create();

bool hashtable_set_load_factor(Hashtable* const hashtable, float loadFactor);

bool hashtable_set(Hashtable* const hashtable, const char* key, const char* value);

const char* hashtable_get(const Hashtable* hashtable, const char* key);

bool hashtable_delete(const Hashtable* hashtable, const char* key);

bool hashtable_destroy(const Hashtable* hashtable);

const HashtableStatistics* hashtable_get_statistics(const Hashtable* hashtable);
