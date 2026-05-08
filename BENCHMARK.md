# Benchmark

## Specs
#### Benchmark
10,000 operations per client, 90% reads (GET) 10% writes (PUT, DEL). 3 character keys and values, no TTL.
```
./bench_client localhost 6379 <clients> 10000 90
```

#### Server
```
./kvserver 6379 <workers> 1000
```

## Results

#### 4 Server Workers
| Concurrent Connections | Operations per Second | Total Time |
| -------- | -------- | --- |
| 1   | 67,635  | 0.1478 Seconds | 
| 4   | 203,117 | 0.1969 Seconds | 
| 16  | 207,315 | 0.7717 Seconds | 
| 64  | 210,509 | 3.0402 Seconds | 

#### 8 Server Workers
| Concurrent Connections | Operations per Second | Total Time |
| -------- | -------- | --- |
| 1   | 66,425  | 0.1505 Seconds | 
| 4   | 205,433 | 0.1947 Seconds | 
| 16  | 312,275 | 0.5124 Seconds | 
| 64  | 303,890 | 2.1060 Seconds | 

#### 12 Server Workers
| Concurrent Connections | Operations per Second | Total Time |
| -------- | -------- | --- |
| 1   | 67,776  | 0.1475 Seconds | 
| 4   | 202,091 | 0.1979 Seconds | 
| 16  | 360,437 | 0.4439 Seconds | 
| 64  | 410,547 | 1.5589 Seconds | 

#### 16 Server Workers
| Concurrent Connections | Operations per Second | Total Time |
| -------- | -------- | --- |
| 1   | 69,235  | 0.1444 Seconds | 
| 4   | 199,290 | 0.2007 Seconds | 
| 16  | 424,170 | 0.3772 Seconds | 
| 64  | 426,305 | 1.5013 Seconds | 

## Analysis
Throughput doesn't scale linearly when only accounting for the number of concurrent connections. Eventually the server worker threads get saturated, like we saw at 16 and 64 clients with 4 workers.

Scaling the number of worker threads up improves performance quite a bit, a 3x worker thread increase corresponds to ~2x operations per second increase. Unfortunatly, my laptop only has 12 cores with hyperthreading, so I belive this is about as good as it will get on my laptop.

I considered temporarily provisioning a powerful cloud VPS to see if the worker thread scaling continued, but I had to draw the line somewhere.