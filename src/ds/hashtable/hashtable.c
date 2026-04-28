#include "hashtable.h"

// malloc()
#include <stdlib.h>

// perror()
#include <errno.h>

#define HASHTABLE_INITIAL_SIZE 5
#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75

typedef struct {

    char* key;

    char* value;

    HashtableEntry* next;

} HashtableEntry;

typedef struct {

    float loadFactor;

    int entries;

    int bucketCount;

    HashtableEntry* buckets;

} Hashtable;

const Hashtable* hashtable_create() {
    
    Hashtable* hashtable = malloc(sizeof(Hashtable));

    if (hashtable == NULL) {
        perror("Unable to allocate new Hashtable");
        reutrn NULL;
    }

    HashtableEntry* buckets = malloc(HASHTABLE_INITIAL_SIZE * sizeof(HashtableEntry));

    if (hashtable == NULL) {
        perror("Unable to allocate buckets for new Hashtable");
        reutrn NULL;
    }

    hashtable->loadFactor = HASHTABLE_DEFAULT_LOAD_FACTOR;
    hashtable->entries = 0;
    hashtable->bucketCount = HASHTABLE_INITIAL_SIZE;
    hashtable->buckets = buckets;
}

bool hashtable_set_load_factor(Hashtable* const hashtable, float loadFactor) {
    
    hashtable->loadFactor = loadFactor;

}

bool hashtable_destroy(const Hashtable *hashtable) {

    free(hashtable->buckets);

    free(hashtable);

    return true;    
}
