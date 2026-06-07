#!/bin/bash

echo "Data_Type,Seconds" > lab2_dataset.csv

echo "Collecting 10 iterations of Contended SPSC..."
for i in {1..10}
do
    TIME=$(./contended_spsc | grep "Elapsed Time:" | awk '{print $3}')
    echo "Contended,$TIME" >> lab2_dataset.csv
done

echo "Collecting 10 iterations of Isolated SPSC..."
for i in {1..10}
do
    TIME=$(./isolated_spsc | grep "Elapsed Time:" | awk '{print $3}')
    echo "Isolated,$TIME" >> lab2_dataset.csv
done

echo "Data collection complete. File saved to lab2_dataset.csv"
cat lab2_dataset.csv
