#include "hashtable.h"

// malloc()
#include <stdlib.h>

// perror()
#include <errno.h>

#include "../../kv.h"

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

    if (buckets == NULL) {
        perror("Unable to allocate buckets for new Hashtable");
        reutrn NULL;
    }

    hashtable->loadFactor = HASHTABLE_DEFAULT_LOAD_FACTOR;
    hashtable->entries = 0;
    hashtable->bucketCount = HASHTABLE_INITIAL_SIZE;
    hashtable->buckets = buckets;

    return hashtable;
}

bool hashtable_set_load_factor(Hashtable* const hashtable, float loadFactor) {
    
    hashtable->loadFactor = loadFactor;

}

bool hashtable_destroy(const Hashtable *hashtable) {

    free(hashtable->buckets);

    free(hashtable);

    return true;    
}

// djb2 hash function
int hash(const char* key, int bucketCount) {

    unsigned long hash = 5381;
    int c;

    for (size_t i = 0; i < MAX_KEY_LEN; i++) {

        char c = key[i];

        if (c == '\0') break;

        hash = ((hash << 5) + hash) + c;
    }

    return (int) (hash % bucketCount);
}