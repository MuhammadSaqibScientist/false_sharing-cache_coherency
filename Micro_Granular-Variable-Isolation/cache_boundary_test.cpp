#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// A dynamically spaced structure to force a specific byte distance between variables
struct ContendingData {
    alignas(64) std::atomic<uint64_t> thread1_target{0};
    
    // This dynamically allocated array acts as our variable-sized padding gap
    uint8_t padding[256]; 
    
    std::atomic<uint64_t> thread2_target{0};
};

void increment_loop(std::atomic<uint64_t>& target, uint64_t iterations) {
    for (uint64_t i = 0; i < iterations; ++i) {
        // Using memory_order_relaxed minimizes structural instruction stalls,
        // isolating purely the microarchitectural hardware cache penalty.
        target.fetch_add(1, std::memory_order_relaxed);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <padding_bytes_to_offset>\n";
        return 1;
    }

    uint32_t padding_offset = std::stoi(argv[1]);
    const uint64_t ITERATIONS = 50'000'000; // 50 Million iterations

    ContendingData data;
    
    // Calculate the absolute distance between the two targets
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(&data.thread1_target);
    // Move the second pointer based on the command line input safely
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(&data.thread2_target) - (256 - padding_offset);
    
    std::atomic<uint64_t>* p1 = &data.thread1_target;
    std::atomic<uint64_t>* p2 = reinterpret_cast<std::atomic<uint64_t>*>(addr2);

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1(increment_loop, std::ref(*p1), ITERATIONS);
    std::thread t2(increment_loop, std::ref(*p2), ITERATIONS);

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print values out cleanly so our automation script can parse them
    std::cout << padding_offset << "," << elapsed_ms << std::endl;

    return 0;
}
