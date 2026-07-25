# Microarchitectural Quantification of False Sharing and Hardware Cache-Line Isolation on Modern x86 Processors

**Author:** Microarchitectural Systems Laboratory  
**Date:** July 2026  
**Subject:** High-Performance Computing (HPC) & Microarchitectural Memory Profiling

---

## Abstract
False sharing occurs when parallel threads on independent CPU cores modify distinct variables residing on the same physical L1 cache line (64 bytes). This study empirically quantifies the throughput degradation induced by the MESI (Modified, Exclusive, Shared, Invalid) cache-coherency protocol under intentional cache contention. By profiling execution across micro-granular memory padding offsets (0B to 128B), we observe an explicit **7.1x execution speedup** and an **85.6% reduction in CPU cycles** once variables achieve 64-byte alignment. Hardware-level trace sampling via Linux `perf c2c` confirms that unpadded access patterns trigger a **77% Hit Modified (HITM) rate**, proving that cache invalidation traffic—rather than algorithmic complexity—is the primary bottleneck in unaligned concurrent data structures.

---

## 1. Introduction & Microarchitectural Background
Modern multi-core processors maintain cache coherency across L1/L2 caches using hardware protocol state machines (such as MESI or MOESI). When Core A writes to a memory address, the interconnect bus issues an **Invalidation Broadcast** to all other cores holding that same 64-byte line. 

If Core B subsequently attempts to read or write an adjacent variable located within that invalidated line, its local L1/L2 cache flags a **Hit Modified (HITM)** event. Core B's execution pipeline stalls while the modified cache line is flushed from Core A's L1 cache to the shared Last Level Cache (LLC) or directly transferred via core-to-core interconnect.

```
        Core 0 (L1 Data Cache)                      Core 1 (L1 Data Cache)
  +-------------------------------+           +-------------------------------+
  |  Variable A [Offset 0x0] (M)  |           |  Variable B [Offset 0x8] (I)  |
  +-------------------------------+           +-------------------------------+
                  |                                           |
                  +=====> MESI Invalidation Broadcast =======>+
                         (Core 1 Pipeline Stalled)
```
---

## 2. Experimental Methodology

### 2.1 Testbed Specifications
* **Architecture:** x86_64 Dual-Core / Quad-Thread Topology
* **L1 Data Cache Line Size:** 64 Bytes
* **Operating System:** Linux Kernel (x86_64)
* **Compiler:** GCC 15 with `-O3` aggressive optimization flag enabled
* **Profiling Tools:** `perf stat` (hardware counters) and `perf c2c` (cache-to-cache interconnect tracing)

### 2.2 Workload & Isolation Constraints
Two parallel threads were spawned to execute 50,000,000 atomic operations (`std::memory_order_relaxed` to isolate raw memory bus latency from compiler instruction barriers). The byte spacing between thread targets was incremented across nine discrete values: 0, 4, 8, 16, 32, 48, 64, 96, and 128 bytes.

---

## 3. Empirical Results & Performance Telemetry

### 3.1 The Microarchitectural Cliff
Telemetry collected across variable alignment intervals reveals a steep performance inflection point occurring precisely at the 64-byte boundary:

| Padding Offset (Bytes) | Execution Time (ms) | L1 D-Cache Load Misses | Total CPU Cycles | Phase Classification |
| :---: | :---: | :---: | :---: | :---: |
| **0** | **3,855** | **1,885,481** | **13,826,327,144** | High Contention |
| **4** | 3,172 | 2,103,421 | 10,488,434,512 | High Contention |
| **8** | 2,303 | 5,074,701 | 10,647,198,747 | High Contention |
| **16** | 2,393 | 4,397,765 | 10,938,856,595 | High Contention |
| **32** | 3,550 | 8,258,070 | 10,055,035,749 | High Contention |
| **48** | 1,909 | 2,052,091 | 11,186,971,777 | High Contention |
| **64** | **543** | **114,141** | **1,991,819,124** | **Hardware Isolated** |
| **96** | 440 | 162,428 | 2,358,863,354 | Hardware Isolated |
| **128** | 429 | 138,850 | 2,014,486,686 | Hardware Isolated |

```
                       THE PERFORMANCE CLIFF
   4000 +--*-------------------------------------------------+
        |   \  (Contention Zone: 0B - 48B)                   |
   3000 |----+--*------*-------*-------*                     |
Time    |                               \                    |
 (ms)   |                                *                   |
   1000 |                                 \  (Isolated)      |
      0 +----------------------------------*----*----*-------+
        0      8      16      32      48   64   96   128
                       Alignment Spacing (Bytes)
```

### 3.2 Hardware Interconnect Analysis (`perf c2c`)
Profiling the unpadded (0-byte) configuration via `perf c2c` captured **47,990 hardware trace records**:
* **Shared Cache Lines Targeted:** `1` (`0x7ffdcebc2240`)
* **Load Local HITM Count:** `18,528` out of 24,050 total load operations (**77.04% HITM rate**)
* **Active Memory Offsets:** `0x0` (Thread 1) and `0x8` (Thread 2)

This confirms that the execution penalty in the contention zone is directly driven by L1 cache line bouncing rather than thread scheduling or algorithmic bottlenecks.

---

## 4. Discussion & Hardware Findings

1. **Sufficiency of 64-Byte Alignment:** Spacing variables by 64 bytes (`alignas(64)`) completely eliminates false sharing. Performance plateaus between 64B and 128B, demonstrating that the hardware adjacent-line prefetcher on this architecture does not cause spatial cache-line bleeding into adjacent lines under relaxed memory ordering.
2. **Resource Consumption Impact:** Transitioning from an unaligned state (0B) to a hardware-isolated state (64B) yields:
   * **85.9% reduction** in wall-clock execution time.
   * **85.6% reduction** in raw CPU clock cycles consumed.
   * **93.9% reduction** in L1 data cache misses.

---

## 5. Conclusion
Hardware cache-line contention is one of the most severe performance bottlenecks in modern parallel systems. By enforcing explicit 64-byte alignment padding (`alignas(64)`), concurrent lock-free algorithms completely eliminate MESI protocol thrashing, achieving maximum hardware pipelining efficiency.
EOF
