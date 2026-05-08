# my-valkey

## Build Instructions
#### Everything
To build everything, run any of the following:
```
make
```
```
make all
```
#### Server
To build just the server, run any of the following:
```
make server
```
```
make kvserver
```
#### Benchmark Client
To build just the benchmark client, run any of the following:
```
make bench
```
```
make bench_client
```

## Run Instructions
I didn't change any binary names from the default Makefile, so shouldn't be any surprises there.

#### Server
The server uses the arguments specified by the instruction document:
```
usage: ./kvserver <port> <num_workers> <num_buckets> [sweeper_interval_ms]
port                TCP port to listen on (1-65535)
num_workers         number of worker threads (>=1)
num_buckets         hash-table bucket count (>=1)
sweeper_interval_ms default 500
```
I wanted to play around with `getopt` but ran out of time.

#### Benchmark Client
The benchmark client also uses the arguments specified by the instruction document:
```
usage: ./bench_client <host> <port> <num_clients> <ops_per_client> <read_pct>
```

## Stage Status
I completed Stages 1 - 4. After 15 million GET/PUT/DEL operations across 15 simultaneous threads with 16 workers, there are no Valgrind leaks. I found it settles in at 1.1 - 1.2 gigabytes of memory usage with my benchmark client implementation. 

TSan reports a race in the `SIGINT` handler because of `printf` and possibly the global `sig_atomic_t` signal variable. This isn't a big deal for a couple reasons:
- This is the only place in prod where `printf` is used, so the call is successfully executed every time
- In the `SIGINT` handler, the atomic shutdown variable only transitions from state `0` to state `1`. Since this is a one way transformation, this function could theoretically be called multiple times and have the exact same effect.
- This function is the only place the atomic shutdown variable is set. It is only read by the hashtable sweeper worker (janitor), and even then it is read with `atomic_load`.

I wanted to create a whole GitHub Actions pipeline to help with tests and a docker deployment, similar to what I did with `my-shell` earlier in the semester. I didn't get around to it, but maybe this summer.

## Design Decisions
#### Lock Granularity
> Why did you use a single table-wide RW lock? What are the concrete tradeoffs versus bucket-level or per-entry locking? What did your benchmark results tell you about whether this was the right choice?

Stage 3 required a single RWLock on the entire hashtable. This is a quick solution, but having a single lock for each bucket is much more efficient. Instead of restricting the entire table to a single thread, only a fraction of the table is restricted. This lets up to _n_ simultaneous writes happen at a time, where _n_ is the number of buckets used. One lock per bucket also means, as long as the keys aren't in the locked bucket, reads can continue as normal.

I though about per entry locking, but decided the added complexity wasn't worth the payoff. Writing a new value wouldn't be so bad, you would only need to lock the single entry you have to edit. Appending to a bucket chain would be more complex, since you would need to acquire the write lock for the tail, forcing readers to wait rigth before the tail to continue. Since I used a doubly linked list, deleting an entry would require locking said entry, its forward, and its backward neighbor to cleanly unlink it from the chain. Readers would need to wait their turn in the chain before finishing the chain read. You'd have to be careful not to get yourself in a deadlock with the lock request ordering, but that shouldn't be too hard by just getting locks in the same order every time.

So you could do all that, or just increase the number of buckets and have close to the same effect. Originally, I had a dynamic hashtable in place, one that would rebalance itself and allocate more buckets and redistribute the contents to be more efficient which could have helped this case. I didn't see a major difference between a single lock vs one lock per bucket, so my bottleneck isn't in the hashtable code.

#### Worker Pool Sizing
> How does performance change with 2, 4, 8, 16 workers? At what point does adding workers stop helping, and why?

The performance benefits from additional worker threads stop scaling between 12 and 16 workers on my machine. I only have 12 cores with hyperthreading and some other stuff running on my laptop, so my hardware cores got saturated. During an intense benchmark I saw my overall CPU utilization hit 120% (probably my boosted clock speed?) which supports my theory.

#### Sweeper Coordination
> How does your sweeper interact with worker threads? What could go wrong if you held the write lock for the entire sweep pass? What did you do instead (or why did you decide it was fine)?

The sweeper (janitor) thread acquires a write lock on each bucket, goes through each entry and deletes it if it's expired, releases the buckets write lock, and continues through each bucket in the hashtable. If it only held read lock then acquired a write lock if it finds an expired entry it would be faster, but there would be a gap between releasing the read lock and acquiring the write lock so an additional check would be necessary to make sure someone didn't delete the entry before we could, or if the entry was updated so it's expiration is farther in the future or removed entirely.

Or, you could just increase the bucket count to minimize bucket sizes, minimizing this problem with minimal memory and almost zero processing overhead.

We just got done implementing Dijkstra's Algorithm in Algorithms, and that uses a priority queue. I wanted to have the janitor thread use a priority queue to keep track of which entries needed to be deleted first, but I ran out of time.

