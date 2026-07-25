# Microarchitectural Quantification of False Sharing and Hardware Cache-Line Isolation on Modern x86 Processors

**Author:** Microarchitectural Systems Laboratory  
**Date:** July 2026  
**Subject:** High-Performance Computing (HPC) & Microarchitectural Memory Profiling

---

## Abstract
False sharing occurs when parallel threads on independent CPU cores modify distinct variables residing on the same physical L1 cache line ($64\text{ bytes}$). This study empirically quantifies the throughput degradation induced by the MESI (Modified, Exclusive, Shared, Invalid) cache-coherency protocol under intentional cache contention. By profiling execution across micro-granular memory padding offsets ($0\text{B}$ to $128\text{B}$), we observe an explicit **$7.1\times$ execution speedup** and an **$85.6\%$ reduction in CPU cycles** once variables achieve $64\text{-byte}$ alignment. Hardware-level trace sampling via Linux `perf c2c` confirms that unpadded access patterns trigger a **$77\%$ Hit Modified (HITM) rate**, proving that cache invalidation traffic—rather than algorithmic complexity—is the primary bottleneck in unaligned concurrent data structures.

---

## 1. Introduction & Microarchitectural Background
Modern multi-core processors maintain cache coherency across L1/L2 caches using hardware protocol state machines (such as MESI or MOESI). When Core $A$ writes to a memory address, the interconnect bus issues an **Invalidation Broadcast** to all other cores holding that same $64\text{-byte}$ line. 

If Core $B$ subsequently attempts to read or write an adjacent variable located within that invalidated line, its local L1/L2 cache flags a **Hit Modified (HITM)** event. Core $B$'s execution pipeline stalls while the modified cache line is flushed from Core $A$'s L1 cache to the shared Last Level Cache (LLC) or directly transferred via core-to-core interconnect.
