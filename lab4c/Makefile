#Name: William Tan
#Email: willtan510@gmail.com
#ID: 104770108

.SILENT:

files=lab4c_tcp.c lab4c_tls.c Makefile README

default:
	gcc -o lab4c_tcp lab4c_tcp.c -lm -lmraa -g -Wall -Wextra
	gcc -o lab4c_tls lab4c_tls.c -lm -lmraa -g -Wall -Wextra -lcrypto -lssl

clean:
	rm -f lab4c-104770108.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czf lab4c-104770108.tar.gz $(files)
