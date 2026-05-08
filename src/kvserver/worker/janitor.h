#pragma once

#include "../hashtable/hashtable.h"
#include <signal.h>

typedef struct JanitorArguments {

    Hashtable* hashtable;
    int scanFrequencyMs;
    volatile sig_atomic_t* shutdown;

} JanitorArguments;

void* janitor_work(void* args);