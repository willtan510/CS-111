#! /usr/local/cs/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... Total number of operations/second for each sync method vs threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

set title "List-1: Throughput of Mutex and Spin-lock Sync Methods"
set xlabel "Threads"
set xrange [0.75:]
set ylabel "Total Throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'
set key left top
plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Mutex' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'Spin-lock' with linespoints lc rgb 'green'



set title "List-2: Per-Operation times for Mutex-Protected List Operations"
set xlabel "Threads"
unset xrange
set xrange[0.75:]
set ylabel "Mean time per operation (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top

plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):($8) title 'Wait-for-lock time' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" using ($2):($7) title 'Avg. completion time' with linespoints lc rgb 'green'
     
set title "List-3: Successful Unprotected Threads and Iterations runs using lists"
set xlabel "Threads"
set xrange [0:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
    "<grep  'list-id-none' lab2b_list.csv" using ($2):($3) with points lc rgb "red" title "No Sync", \
    "<grep  'list-id-m' lab2b_list.csv" using ($2):($3) with points lc rgb "blue" title "Mutex", \
    "<grep  'list-id-s' lab2b_list.csv" using ($2):($3) with points lc rgb "green" title "Spin"
    

set title "List-4: Throughput of Mutex Synchronized Partitioned Lists"
set xlabel "Threads"
set xrange [0:]
set ylabel "Aggregated Throughput(Operations/Second)"
set logscale y
set key left top
set output 'lab2b_4.png'
plot \
    "< grep 'list-none-m,[0-9][2]\\?,1000,1,' lab2b_list.csv" using ($2):(1000000000 /($7)) title '--lists=1' with linespoints lc rgb "red", \
    "< grep 'list-none-m,[0-9][2]\\?,1000,4,' lab2b_list.csv" using ($2):(1000000000 /($7)) title '--lists=4' with linespoints lc rgb "blue", \
    "< grep 'list-none-m,[0-9][2]\\?,1000,8,' lab2b_list.csv" using ($2):(1000000000 /($7)) title '--lists=8' with linespoints lc rgb "green", \
     "< grep 'list-none-m,[0-9][2]\\?,1000,16,' lab2b_list.csv" using ($2):(1000000000 / ($7)) title '--lists=16' with linespoints lc rgb "violet"
    

set title "List-4: Throughput of Spinlock Synchronized Partitioned Lists"
set xlabel "Threads"
set xrange [0:]
set ylabel "Aggregated Throughput (Operations/Second)"
set logscale y
set output 'lab2b_5.png'
plot \
    "< grep 'list-none-s,[0-9][2]\\?,1000,1,' lab2b_list.csv" using ($2):(1000000000 / ($7)) title '--lists=1' with linespoints lc rgb 'blue', \
    "< grep 'list-none-s,[0-9][2]\\?,1000,4,' lab2b_list.csv" using ($2):(1000000000 / ($7)) title '--lists=4' with linespoints lc rgb 'green', \
    "< grep 'list-none-s,[0-9][2]\\?,1000,8,' lab2b_list.csv" using ($2):(1000000000 / ($7)) title '--lists=8' with linespoints lc rgb 'orange', \
    "< grep 'list-none-s,[0-9][2]\\?,1000,16,' lab2b_list.csv" using ($2):(1000000000 / ($7)) title '--lists=16' with linespoints lc rgb 'red'
