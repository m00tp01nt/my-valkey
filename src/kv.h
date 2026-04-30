/*
 * kv.h -- Mini-KV server: shared declarations
 *
 * Project 2, CprE 3080, Spring 2026
 *
 * You may modify this file. It is provided as a starting point, not a rigid
 * interface. If your design benefits from additional fields or types, add them.
 */
#ifndef KV_H
#define KV_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

// Claude generated, supposed to let me use the defined constants in string macros
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/* -------- Protocol constants (do NOT change) ---------------------------- */

#define MAX_KEY_LEN         256
#define MAX_VAL_LEN         256
#define MAX_OPERATION_LEN   5
#define MAX_TTL_LEN         5
#define MAX_SPACE_COUNT     3
#define MAX_TOKEN_COUNT     4
#define MAX_TTL             86400 // 24 hours

#define I_TERMINATOR '\n'
#define I_DELIMITER ' '

// Tighter max line length
#define MAX_LINE_LEN        (MAX_OPERATION_LEN + MAX_KEY_LEN + MAX_VAL_LEN + MAX_TTL_LEN + MAX_SPACE_COUNT + 64)

// Response
#define RESPONSE_EPILOGUE_C '\n'
#define RESPONSE_EPILOGUE_S "\n"
#define MAX_RESPONSE_BODY_LENGTH 128

// Operation usage
#define O_USAGE_PREAMBLE "Usage: "
#define O_USAGE_EPILOGUE "\\n"
#define O_GET_USAGE O_USAGE_PREAMBLE "GET <key>" O_USAGE_EPILOGUE
#define O_GET_MIN_ARGS 1
#define O_GET_MAX_ARGS 1

#define O_PUT_USAGE O_USAGE_PREAMBLE "PUT <key> <value>" O_USAGE_EPILOGUE
#define O_PUT_TTL_USAGE O_USAGE_PREAMBLE "PUT <key> <value> <ttl>" O_USAGE_EPILOGUE
#define O_PUT_MIN_ARGS 2
#define O_PUT_MAX_ARGS 3

#define O_DEL_USAGE O_USAGE_PREAMBLE "DEL <key>" O_USAGE_EPILOGUE
#define O_DEL_MIN_ARGS 1
#define O_DEL_MAX_ARGS 1

#define O_STATS_USAGE O_USAGE_PREAMBLE "STATS" O_USAGE_EPILOGUE
#define O_STATS_MIN_ARGS 0
#define O_STATS_MAX_ARGS 0

#define O_QUIT_USAGE O_USAGE_PREAMBLE "QUIT" O_USAGE_EPILOGUE
#define O_QUIT_MIN_ARGS 0
#define O_QUIT_MAX_ARGS 0

/* -------- Your types go here -------------------------------------------- */

/*
 * TODO (Stage 1): Define your hash-table entry and bucket types.
 *
 * TODO (Stage 2): Define your work-queue type (bounded FIFO of int fds).
 *
 * TODO (Stage 3): Add an rwlock to your table type.
 *
 * TODO (Stage 4): Add expiration timestamp to entries; declare the sweeper
 *                 thread function.
 */

/* -------- Function prototypes you will likely want ---------------------- */

/* Protocol / connection handling (Stage 1) */
void handle_client(int);        /* loop: read line, parse, reply */

/* Hash-table operations (Stage 1, made thread-safe in Stage 3) */
/*   Return 0 on success, -1 on not-found / error. */
/*   You design the full signatures -- these are just suggestions. */
/* int  kv_get(const char *key, char *out_val, size_t out_cap); */
/* int  kv_put(const char *key, const char *val, int ttl_seconds); */
/* int  kv_del(const char *key); */

#endif /* KV_H */
