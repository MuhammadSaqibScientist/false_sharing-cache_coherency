# Empirical Analysis of False Sharing on Intel i5-4200U

## Experimental Results
| Metric | Contended Binary | Aligned Binary | Improvement |
| :--- | :--- | :--- | :--- |
| **Execution Time** | 3.099s | 2.646s | ~14.6% Faster |
| **L1 D-Cache Misses**| 108,291,330 | 299,348 | 99.7% Reduction |
| **Total CPU Cycles** | 14.13 Billion | 12.15 Billion | 1.98 Billion Cycles Saved |

## Hardware Insights
The results confirm that cache-line contention triggers significant invalidation traffic under the MESI protocol...


## command 
> sudo sysctl -w kernel.perf_event_paranoid=-1
