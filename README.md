## Empirical Case Study: Mitigating False Sharing Cache Invalidation

This laboratory module analyzes the real-world performance impact of false sharing across independent processor core execution pipelines using hardware performance counters (`perf`). 

### Hardware Performance Telemetry Analysis

By evaluating the compiled execution sequences over a 10-trial automated testing campaign, we mapped the following architecture behaviors:

| Metric | Contended Execution (`./contended_test`) | Cache-Aligned Execution (`./aligned_test`) | Delta / Optimization Factor |
| --- | --- | --- | --- |
| **Avg Execution Time** | ~3.15 Seconds | ~2.71 Seconds | **~14% Performance Speedup** |
| **Total CPU Cycles** | ~14,200,000,000 | ~12,400,000,000 | **~1.8 Billion Cycles Saved** |
| **L1 Data Cache Misses**| **~104,399,360** | **~343,401** | **99.6% Reduction in L1 Jitter** |

### Critical Takeaway
The high volume of L1 cache load misses in the contended run acts as direct evidence of **cache coherence protocol overhead (MESI)**. Because multiple threads concurrently mutated distinct variables sharing a single 64-byte cache line, cache lines were forced to broadcast invalidation signals repeatedly across the interconnect bus. Aligning our data parameters cleanly to memory block boundaries solved the contention completely at the hardware layer.
