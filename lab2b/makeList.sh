#!/bin/bash

./lab2_list --threads=1  --iterations=1000 --sync=m --lists=4 > lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv

./lab2_list --threads=1  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv	
./lab2_list --threads=8  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv


for sync in m s; do
    for threads in 1 2 4 8 12 16 24; do
        echo "Sync type $sync: $threads threads, 1000 iterations"
        ./lab2_list --threads=$threads --iterations=1000 --sync=$sync >> lab2b_list.csv
    done
done

for threads in 1 4 8 12 16; do
    for iterations in 1 2 4 8 16; do
        echo "No sync, $threads threads, $iterations iterations, yield=id, 4 lists"
        ./lab2_list --threads=$threads --iterations=$iterations --yield=id --lists=4 >>lab2b_list.csv
    done
done

for synctype in m s; do
    for threads in 1 4 8 12 16; do
        for iterations in 10 20 40 80; do
            echo "Sync type: $synctype , $threads threads, $iterations iterations, yield=id, 4 lists"
            ./lab2_list --threads=$threads --iterations=$iterations --yield=id --sync=$synctype --lists=4 >>lab2b_list.csv
        done
    done
done

