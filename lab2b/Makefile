#NAME: William Tan
#EMAIL: Willtan510@gmail.com 


.SILENT:

files= SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png README makeList.sh graph.gp

build:
	gcc -lrt -o lab2_list lab2_list.c SortedList.c -pthread -g -Wall -Wextra

tests: clean build
	./makeList.sh

profile: clean build
	rm -f ./rawprofile
	LD_PRELOAD=/u/cs/ugrad/tanw/lib/libprofiler.so.0 CPUPROFILE=./rawprofile ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./rawprofile > profile.out
	pprof --list=threadList ./lab2_list ./rawprofile >> profile.out
	rm -f ./rawprofile

graphs: tests
	./graph.gp

dist: clean build graphs profile
	tar -czf lab2b-104770108.tar.gz $(files)

clean:
	rm -f lab2_list lab2b-104770108.tar.gz
