#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// Define the number of operations each thread will perform (1 billion loops)
const uint64_t ITERATIONS = 1000000000ULL;

// The core configuration structure
struct ThreadData {
#ifdef ALIGN_CACHE
    // Aligned Version: Variables are forced onto completely separate 64-byte cache lines
    alignas(64) uint64_t value{0};
#else
    // Contended Version: Variables sit side-by-side in memory (sharing a cache line)
    uint64_t value{0};
#endif
};

// The execution target for our threads
void increment_worker(ThreadData& data) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        // This simple addition forces cache-coherency logic to constantly signal between cores
        asm volatile("" : "+m"(data.value));
        data.value++;
    }
}

int main() {
    // We launch 2 threads since your i5-4200U has 2 physical cores
    const int NUM_THREADS = 2;
    std::vector<ThreadData> data_array(NUM_THREADS);
    std::vector<std::thread> threads;

    std::cout << "Starting scientific experiment with " << NUM_THREADS << " threads..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch workers targeting separate array entries
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(increment_worker, std::ref(data_array[i]));
    }

    // Wait for all processing to finish
    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Experiment finalized successfully." << std::endl;
    std::cout << "Execution Time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
