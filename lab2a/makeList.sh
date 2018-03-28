#!/bin/bash

for threads in 1; do
    for iterations in 10 100 1000 10000 20000; do
        echo "No sync: 1 thread, $iterations iterations"
        ./lab2_list --threads=$threads --iterations=$iterations >> lab2_list.csv
    done
done

for threads in 2 4 8 12; do
    for iterations in 1 10 100 1000; do
        ./lab2_list --threads=$threads --iterations=$iterations >> lab2_list.csv
    done
done

for threads in 2 4 8 12; do
    for iterations in 1 2 4 8 16 32; do
        echo "Yield: $threads threads, $iterations iterations"
        ./lab2_list --threads=$threads --iterations=$iterations >> lab2_list.csv
        ./lab2_list --threads=$threads --iterations=$iterations --yield=i >> lab2_list.csv
        ./lab2_list --threads=$threads --iterations=$iterations --yield=d >> lab2_list.csv
        ./lab2_list --threads=$threads --iterations=$iterations --yield=il >> lab2_list.csv
        ./lab2_list --threads=$threads --iterations=$iterations --yield=dl >> lab2_list.csv
    done
done

for sync in m s; do
    for threads in 1 2 4 8 12 16 24; do
        echo "Sync type $sync: $threads threads, 1000 iterations"
        ./lab2_list --threads=$threads --iterations=1000 --sync=$sync >> lab2_list.csv
    done
done


for threads in 12; do
    for sync in m s; do
        for iterations in  32; do
            echo "Sync type $sync: $threads threads, $iterations iterations"
            ./lab2_list --threads=$threads --iterations=$iterations --sync=$sync >> lab2_list.csv
            ./lab2_list --threads=$threads --iterations=$iterations --yield=i --sync=$sync >> lab2_list.csv
            ./lab2_list --threads=$threads --iterations=$iterations --yield=d --sync=$sync>> lab2_list.csv
            ./lab2_list --threads=$threads --iterations=$iterations --yield=il --sync=$sync>> lab2_list.csv
            ./lab2_list --threads=$threads --iterations=$iterations --yield=dl --sync=$sync>> lab2_list.csv
        done
    done
done


