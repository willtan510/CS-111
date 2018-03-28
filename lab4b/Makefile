#Name: William Tan
#Email: willtan510@gmail.com
#ID: 104770108

.SILENT:

files=lab4b.c Makefile README tester.sh

default:
	gcc -o lab4b lab4b.c -lm -lmraa -g -Wall -Wextra
	
check:
	./tester.sh

clean:
	rm -f lab4b-104770108.tar.gz lab4b

dist:
	tar -czf lab4b-104770108.tar.gz $(files)
