#!/bin/bash

# Compile the benchmark
g++ -O3 -pthread cache_boundary_test.cpp -o cache_boundary_test

# Initialize the raw data file
OUTPUT_FILE="cache_cliff_results.csv"
echo "Padding_Bytes,Duration_MS,L1_Dcache_Misses,Cycles" > $OUTPUT_FILE

# List of byte steps to test
PADDING_SIZES=(0 4 8 16 32 48 64 96 128)

echo "Starting scientific data sweep..."

for bytes in "${PADDING_SIZES[@]}"
do
    echo "Testing alignment spacing: ${bytes} bytes..."
    
    # Use perf stat to capture raw hardware counters for each specific alignment
    # We run it 3 times to get an average or standard baseline
    PERF_DATA=$(sudo perf stat -e L1-dcache-load-misses,cycles -x , ./cache_boundary_test $bytes 2>&1)
    
    # Parse out the duration generated from our application output
    APP_OUTPUT=$(./cache_boundary_test $bytes)
    DURATION=$(echo "$APP_OUTPUT" | cut -d',' -f2)
    
    # Extract the counters from the comma-separated perf output
    L1_MISSES=$(echo "$PERF_DATA" | grep "L1-dcache-load-misses" | cut -d',' -f1)
    CYCLES=$(echo "$PERF_DATA" | grep "cycles" | cut -d',' -f1)
    
    # Record everything to our scientific CSV ledger
    echo "${bytes},${DURATION},${L1_MISSES},${CYCLES}" >> $OUTPUT_FILE
done

echo "Experiment sweep finished! Raw data saved to: ${OUTPUT_FILE}"
cat $OUTPUT_FILE

