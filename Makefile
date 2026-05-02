# Mini-KV Makefile -- CprE 308/3080 Project 2
#
# Targets:
#   make             build kvserver
#   make bench       build bench_client
#   make all         both
#   make clean       remove build artifacts
#   make tsan        build with ThreadSanitizer (Stage 3+ debugging)

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -g -pthread
CFLAGS  += -Wswitch
CFLAGS  += -Wwrite-strings
CFLAGS  += -Wcast-qual
# CFLAGS  += -O2
CFLAGS  += -O0

LDFLAGS = -pthread

COMMON_SRCS =  src/common/operation.c
COMMON_SRCS += src/common/response.c
COMMON_SRCS += src/common/logger.c
COMMON_SRCS += src/common/command.c
COMMON_SRCS += src/common/token.c

SERVER_SRCS =  src/kvserver/kvserver.c
SERVER_SRCS += src/kvserver/hashtable/hashtable.c
SERVER_SRCS += src/kvserver/queue/queue.c
SERVER_SRCS += src/kvserver/input/input.c
SERVER_SRCS += src/kvserver/worker/worker.c

BENCH_SRCS  =  src/benchmark/bench_client.c
BENCH_SRCS  += src/benchmark/worker/worker.c

SERVER_BIN = kvserver
BENCH_BIN  = bench_client

.PHONY: all bench clean tsan

all: $(SERVER_BIN) $(BENCH_BIN)

bench: $(BENCH_BIN)

$(SERVER_BIN): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(COMMON_SRCS) $(SERVER_SRCS) $(LDFLAGS)

$(BENCH_BIN): $(BENCH_SRCS)
	$(CC) $(CFLAGS) -o $@ $(COMMON_SRCS) $(BENCH_SRCS) $(LDFLAGS)

# Use this during development of Stage 3 and 4 to catch data races.
# Run the server normally -- TSan reports races on stderr.
tsan: CFLAGS += -fsanitize=thread -O1
tsan: LDFLAGS += -fsanitize=thread
tsan: clean $(SERVER_BIN)

clean:
	rm -f $(SERVER_BIN) $(BENCH_BIN) *.o
