#Name: William Tan
#Email: willtan510@gmail.com
#ID: 104770108

.SILENT:

files=lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png makeAdd.sh makeList.sh README

build: 
	gcc -o lab2_add lab2_add.c -pthread -Wall -Wextra
	gcc -o lab2_list lab2_list.c SortedList.c -pthread -Wall -Wextra

dist:
	tar -czf lab2a-104770108.tar.gz $(files)

tests: 
	chmod 777 makeAdd.sh
	./makeAdd.sh
	chmod 777 makeList.sh
	./makeList.sh

graphs:
	./lab2_add.gp
	./lab2_list.gp

clean:
	rm -f lab2a-104770108.tar.gz *.png *.csv lab2_add lab2_list
