/*
 * bench_client.c -- Mini-KV concurrent benchmark client
 *
 * Project 2, CprE 3080, Spring 2026
 *
 * YOU write this file. The scaffolding here is intentionally minimal:
 * an argument parser and nothing else. Your job is to fill in the rest.
 *
 * Usage:
 *   ./bench_client <host> <port> <num_clients> <ops_per_client> <read_pct>
 *
 * Requirements (from the spec):
 *   - Spawn <num_clients> threads.
 *   - Each thread opens its own TCP connection to <host>:<port>.
 *   - Each thread issues <ops_per_client> operations.
 *   - <read_pct> percent of ops are GETs; the rest are PUTs.
 *   - Keys drawn from a small pool (~1000 keys) so GETs actually hit.
 *   - Report total wall-clock time and overall throughput (ops/sec).
 *
 * Hints:
 *   - Use clock_gettime(CLOCK_MONOTONIC, ...) to measure elapsed time.
 *   - Each thread needs its own rand_r() seed to avoid serialization on
 *     the global rand() lock.
 *   - Read the server's reply line-by-line; don't assume one TCP packet
 *     per command.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "../common/bool.h"
#include "worker/worker.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "usage: %s <host> <port> <num_clients> <ops_per_client> <read_pct>\n",
        prog);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        usage(argv[0]);
        return 1;
    }

    const char *host      = argv[1];
    int port              = atoi(argv[2]);
    int num_clients       = atoi(argv[3]);
    int ops_per_client    = atoi(argv[4]);
    int read_pct          = atoi(argv[5]);

    if (port <= 0 || num_clients < 1 || ops_per_client < 1 ||
        read_pct < 0 || read_pct > 100) {
        usage(argv[0]);
        return 1;
    }

    unsigned int baseSeed = rand();

    WorkerArguments arguments[num_clients];
    pthread_t threads[num_clients];

    for (int i = 0; i < num_clients; i++) {    
        arguments[i].host = host;
        arguments[i].port = port;
        arguments[i].readPercent = read_pct;
        arguments[i].totalOperations = ops_per_client;
        arguments[i].seed = baseSeed + i;
    }

    struct timespec start, end;
    
    printf("\n");
    printf("Running...");

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_clients; i++) 
        pthread_create(&threads[i], NULL, stress_test, &arguments[i]);

    for (int i = 0; i < num_clients; i++) 
        pthread_join(threads[i], NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                    (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Done\n");
    printf("\n");
    printf("Results:\n");
    printf("\tTotal Time: %lf sec\n", elapsed);
    printf("\tTotal Operations: %d\n", (num_clients * ops_per_client));
    printf("\tOperations per Second: %lf\n", (num_clients * ops_per_client) / elapsed);
    printf("\n");

    /* TODO:
     *   1. Spawn num_clients pthreads.
     *   2. Each thread: connect to <host>:<port>, run ops_per_client
     *      operations mixing GETs and PUTs per read_pct.
     *   3. Join all threads.
     *   4. Compute and print total elapsed time and total ops/sec.
     */

    return 0;
}
