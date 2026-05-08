#include "janitor.h"

#include <time.h>
#include <stdatomic.h>

#include "../hashtable/hashtable.h"

void* janitor_work(void* args) {

    JanitorArguments* arguments = (JanitorArguments*) args;

    struct timespec ts;
    ts.tv_sec = (arguments->scanFrequencyMs / 1000);
    ts.tv_nsec = (arguments->scanFrequencyMs % 1000) * 1000000;

    while (!atomic_load(arguments->shutdown))  {
        nanosleep(&ts, NULL);
        hashtable_job_janitor(arguments->hashtable);
    }

    return NULL;
}
