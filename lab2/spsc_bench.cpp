#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

// The capacity MUST be a power of 2 for our bitwise wrapping optimization
constexpr size_t BUFFER_CAPACITY = 1024;
constexpr size_t BUFFER_MASK = BUFFER_CAPACITY - 1;
constexpr uint64_t TOTAL_OPERATIONS = 50000000ULL; // 50 Million data transfers

struct SPSCQueue {
#ifdef ISOLATE_CACHE
    // --- OPTIMIZED MODE ---
    // Force head and tail onto completely independent 64-byte cache lines
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
    alignas(64) int storage[BUFFER_CAPACITY]{0};
#else
    // --- CONTENDED MODE ---
    // Variables sit packed together, colliding inside the exact same cache line
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
    int storage[BUFFER_CAPACITY]{0};
#endif

    // PRODUCER THREAD OPERATION
    bool push(int item) {
        // Load variables using explicit acquire-release semantics
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t current_tail = tail.load(std::memory_order_acquire);

        // Check if queue is full
        if ((current_head - current_tail) == BUFFER_CAPACITY) {
            return false; // Backpressure: Producer must retry
        }

        // Bitwise AND (& BUFFER_MASK) wraps the pointer in 1 CPU cycle instead of costly division (%)
        storage[current_head & BUFFER_MASK] = item;
        
        // Release semantics ensure the data write is committed before head increments
        head.store(current_head + 1, std::memory_order_release);
        return true;
    }

    // CONSUMER THREAD OPERATION
    bool pop(int& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t current_head = head.load(std::memory_order_acquire);

        // Check if queue is empty
        if (current_head == current_tail) {
            return false; // Queue empty: Consumer must retry
        }

        item = storage[current_tail & BUFFER_MASK];
        
        // Release semantics ensure reading is finished before tail increments
        tail.store(current_tail + 1, std::memory_order_release);
        return true;
    }
};

// Global queue instantiation
SPSCQueue queue;

// The thread functions
void producer_worker() {
    for (uint64_t i = 0; i < TOTAL_OPERATIONS; ++i) {
        // Keep retrying until there is space in the buffer
        while (!queue.push(static_cast<int>(i))) {
            // Spin-lock loop
            asm volatile("" : : : "memory");
        }
    }
}

void consumer_worker() {
    int extracted_value = 0;
    for (uint64_t i = 0; i < TOTAL_OPERATIONS; ++i) {
        // Keep retrying until data becomes available
        while (!queue.pop(extracted_value)) {
            // Spin-lock loop
            asm volatile("" : : : "memory");
        }
    }
}

int main() {
#ifdef ISOLATE_CACHE
    std::cout << "[SCIENCE INFO] Running OPTIMIZED mode (Cache lines isolated)." << std::endl;
#else
    std::cout << "[SCIENCE INFO] Running CONTENDED mode (False Sharing active)." << std::endl;
#endif

    std::cout << "Spawning parallel Producer and Consumer threads across cores..." << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch threads concurrently 
    std::thread producer(producer_worker);
    std::thread consumer(consumer_worker);

    producer.join();
    consumer.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Execution Finalized Cleanly." << std::endl;
    std::cout << "Elapsed Time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
