#define _POSIX_C_SOURCE 200809L

/*
 * kvserver.c -- Mini-KV server entry point
 *
 * Project 2, CprE 3080, Spring 2026
 *
 * Starter scaffolding: this file gives you a working TCP listener and an
 * argument parser. Everything else -- accept loop, protocol, hash table,
 * thread pool, RW locking, TTL sweeper -- is yours to write.
 *
 * Build: run `make` in this directory. See the provided Makefile.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "input/input.h"
#include "input/problem.h"

#include "../common/logger.h"
#include "../common/bool.h"

#include "hashtable/hashtable.h"
#include "queue/queue.h"

#include "worker/worker.h"
#include "worker/janitor.h"

/* -------- Globals ------------------------------------------------------- */

static volatile sig_atomic_t g_shutdown = 0;
static atomic_uint connections = 0;

// TSan reports a race which is technically true, however:
//      1) This is a one way transformation between two states
//      2) This is the only place that shutdown is changed
//      3) The Janitor is the only other place where this is read,
//          since we are shutting down it doesn't really matter. 
static void sigint_handler(int signal) {
    (void) signal;
    if (!g_shutdown)
        printf(SHUTDOWN_MESSAGE_NORMAL);
    else {
        printf(SHUTDOWN_MESSAGE_ABORT);
        _exit(0);
    }

    g_shutdown = 1;
}

/* -------- Socket helpers ------------------------------------------------ */

/* Create a listening TCP socket bound to the given port. Returns fd or -1. */
static int make_listen_socket(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }
    if (listen(fd, 64) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }
    return fd;
}

/* -------- Entry point --------------------------------------------------- */

static void usage(const char *prog) {
    fprintf(stderr,
        "usage: %s <port> <num_workers> <num_buckets> [sweeper_interval_ms]\n"
        "   port                TCP port to listen on (1-65535)\n"
        "   num_workers         number of worker threads (>=1)\n"
        "   num_buckets         hash-table bucket count (>=1)\n"
        "   sweeper_interval_ms default 500\n",
        prog);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        usage(argv[0]);
        return 1;
    }
    int port         = atoi(argv[1]);
    int num_workers  = atoi(argv[2]);
    int num_buckets  = atoi(argv[3]);
    int sweeper_ms   = (argc == 5) ? atoi(argv[4]) : 500;

    if (port <= 0 || port > 65535 || num_workers < 1 ||
        num_buckets < 1 || sweeper_ms <= 0) {
        usage(argv[0]);
        return 1;
    }

    /* Install Ctrl-C handler for clean shutdown. */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Ignore SIGPIPE: writes to closed sockets should fail with EPIPE, not
     * kill the server. */
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = make_listen_socket(port);
    if (listen_fd < 0) return 1;

    Queue* queue;
    Hashtable* hashtable;

    queue = queue_create(1024);
    hashtable = hashtable_create(num_buckets);

    WorkerAguments workerArgs = {
        .connections = &connections,
        .hashtable = hashtable,
        .queue = queue,
    };

    pthread_t threads[num_workers];
    for (int i = 0; i < num_workers; i++) {
        pthread_create(
            &threads[i],
            NULL,
            kvserver_work,
            (void*)(&workerArgs)
        );
    }

    JanitorArguments janitorArguments;
    janitorArguments.hashtable = hashtable;
    janitorArguments.scanFrequencyMs = sweeper_ms;
    janitorArguments.shutdown = &g_shutdown;

    pthread_t janitor;
    pthread_create(
        &janitor,
        NULL,
        janitor_work,
        (void*)(&janitorArguments)
    );

    fprintf(stderr,
        "kvserver: listening on port %d "
        "(workers=%d, buckets=%d, sweeper=%dms)\n",
        port, num_workers, num_buckets, sweeper_ms);

    while (!g_shutdown) {
        int conn = accept(listen_fd, NULL, NULL);
        if (conn < 0) {

            if (errno == EINTR) break;
            perror("accept");
            break;

        }
        queue_add(queue, (QueueEntry){
            .fd = conn
        });
    }
    close(listen_fd);
    queue_drain(queue);

    for (int i = 0; i < num_workers; i++)
        pthread_join(threads[i], NULL);

    pthread_join(janitor, NULL);

    hashtable_destroy(hashtable);
    queue_destroy(queue);

    printf("Stopped\n");
    return 0;
}
