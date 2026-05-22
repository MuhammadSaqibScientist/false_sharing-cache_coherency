# Empirical Analysis of False Sharing and Cache Coherency Overheads in Multi-Core Systems

## 1. Abstract
This research quantifies the architectural overhead of the MESI (Modified, Exclusive, Shared, Invalid) cache coherency protocol under false sharing conditions. Using an Intel Core i5-4200U (Haswell architecture, 2 physical cores, 4 logical threads via Hyper-Threading), we isolate execution behaviors when parallel worker threads update adjacent vs. padded variables within the standard 64-byte L1 Data cache line boundary. Our findings show that forcing pointer isolation yields a **99.7% reduction in L1 data cache load misses** and cuts overall runtime overhead by **~14.2%**.

## 2. Methodology & Experimental Design
The experiment spawns $N=2$ concurrent POSIX threads utilizing standard C++ primitives (`std::thread`). Each thread executes exactly $10^9$ increments on an assigned counter within a struct array.
* **Contended Variant:** Counters are tightly packed sequential 64-bit unsigned integers (`uint64_t`), positioning multiple thread targets within the same physical 64-byte cache line.
* **Aligned Variant:** Pointer targets are explicitly shifted using the compile-time specifier `alignas(64)`, ensuring individual counters occupy separate cache structures.

Compiler optimizations were controlled using GCC inline assembly wrappers (`asm volatile`) to guarantee rigorous loop generation without statement elimination or register allocation shortcuts.

## 3. Hardware Profile
* **CPU:** Intel(R) Core(TM) i5-4200U CPU @ 1.60GHz (Max 2.60GHz)
* **Microarchitecture:** Haswell (x86_64)
* **Caches:** L1d: 64 KiB (per-core isolated), L2: 512 KiB, L3: 3 MiB Shared.
* **Profiling Framework:** Linux Kernel Hardware Performance Counters via `perf-stat`.

## 4. Empirical Evaluation Matrix
Averaged across 10 randomized sequential trials:

| Metric Evaluation | Contended Configuration | Aligned Configuration | Net Structural Shift |
| :--- | :--- | :--- | :--- |
| **Mean Execution Time** | 3.120 seconds | 2.669 seconds | **14.45% Latency Reduction** |
| **Total Pipeline Cycles** | 14.18 Billion | 12.23 Billion | **1.95 Billion Cycles Reprieved** |
| **Instruction Throughput**| 6.01 Billion | 6.01 Billion | Identical Work Profile |
| **L1 D-Cache Load Misses**| 107,249,974 | 374,482 | **99.65% Cache Miss Elimination** |

## 5. Architectural Inference
When variables reside inside a shared cache line, parallel modification forces alternating cores to repeatedly broadcast validation state changes via the hardware interconnect fabric. The local cache copy transitions to an **Invalid (I)** state on Core A when modified by Core B, necessitating a continuous pipeline stall while the cache line is re-fetched from higher levels. Aligning data structures on structural cache boundaries prevents mutual cache invalidation, maintaining local L1 cache stability in a **Modified (M)** state and allowing execution units to saturate vector additions without system bus interference.

## 6. How to Reproduce
```bash
# Clone the repository and navigate to Lab 1
g++ -O2 false_sharing_bench.cpp -o contended_test
g++ -O2 -DALIGN_CACHE false_sharing_bench.cpp -o aligned_test
chmod +x run_experiments.sh
./run_experiments.sh
python3 plot_results.py