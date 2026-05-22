#!/bin/bash

echo "=================================================="
echo "Starting Automated Hardware Measurement Campaign"
echo "=================================================="

# Create an output directory for our raw data files
mkdir -p results

echo "Trial,Type,Seconds,Cycles,Instructions,L1_Misses" > results/hardware_dataset.csv

# Run 10 iterations for the Contended binary
for i in {1..10}
do
    echo "Running Contended Test - Trial $i..."
    # We run perf stat and redirect its output cleanly to a temporary file
    perf stat -e cycles,instructions,cache-misses,L1-dcache-load-misses ./contended_test 2> temp.log
    
    # Extract the numbers using standard Linux text processing tools
    cycles=$(grep "cycles" temp.log | awk '{print $1}' | tr -d ',')
    instr=$(grep "instructions" temp.log | awk '{print $1}' | tr -d ',')
    l1_miss=$(grep "L1-dcache-load-misses" temp.log | awk '{print $1}' | tr -d ',')
    seconds=$(grep "time elapsed" temp.log | awk '{print $1}')
    
    # Append the row straight into our CSV database
    echo "$i,contended,$seconds,$cycles,$instr,$l1_miss" >> results/hardware_dataset.csv
done

# Run 10 iterations for the Aligned binary
for i in {1..10}
do
    echo "Running Aligned Test - Trial $i..."
    perf stat -e cycles,instructions,cache-misses,L1-dcache-load-misses ./aligned_test 2> temp.log
    
    cycles=$(grep "cycles" temp.log | awk '{print $1}' | tr -d ',')
    instr=$(grep "instructions" temp.log | awk '{print $1}' | tr -d ',')
    l1_miss=$(grep "L1-dcache-load-misses" temp.log | awk '{print $1}' | tr -d ',')
    seconds=$(grep "time elapsed" temp.log | awk '{print $1}')
    
    echo "$i,aligned,$seconds,$cycles,$instr,$l1_miss" >> results/hardware_dataset.csv
done

# Clean up temporary logs
rm temp.log

echo "=================================================="
echo "Experiment Complete! Data saved to results/hardware_dataset.csv"
echo "=================================================="
