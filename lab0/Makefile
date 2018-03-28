#Name: William Tan
#Email: willtan510@gmail.com 
#ID: 104770108

files = lab0.c Makefile backtrace.png breakpoint.png README

all: 
	@gcc -o lab0 -g ./lab0.c -Wall -Wextra

dist: 
	@tar -czf lab0-104770108.tar.gz $(files)

clean:
	rm -f lab0-104770108.tar.gz lab0

check: all check_0 check_1 check_2access check_2exist check_3 check_catch check_diff


check_0:
	@echo "This should become the text of output.txt" > input.txt; \
	touch output.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	if [ $$? -ne 0 ]; \
	then \
	echo "Return value was not 0, so program did not finish correctly"; \
	fi; \
	rm -f input.txt output.txt

check_1:
	@./lab0 --wrong &>/dev/null; \
	if [ $$? -ne 1 ]; \
	then \
	echo "Invalid option that was not caught by program"; \
	fi

check_2access:
	@touch input.txt; \
	chmod 000 input.txt; \
	./lab0 --input=input.txt &>/dev/null; \
	if [ $$? -ne 2 ]; \
	then \
	echo "Error in recognizing that the file could not be read"; \
	fi; \
	rm -f input.txt; \

check_2exist:
	@./lab0 --input=nwnfelsgnrl &>/dev/null; \
	if [ $$? -ne 2 ]; \
	then \
	echo "Error in recognizing that file does not exist"; \
	fi

check_3:
	@touch output.txt; \
	chmod 000 output.txt; \
	./lab0 --output=output.txt &>/dev/null; \
	if [ $$? -ne 3 ]; \
	then \
	echo "Error recognizng file cannot be written into"; \
	fi; \
	rm -f output.txt; \

check_catch:
	@./lab0 --segfault --catch &>/dev/null;  \
	if [ $$? -ne 4 ]; \
	then \
	echo "Error with catch option"; \
	fi

check_diff:
	@echo "This is input" > input.txt; \
	./lab0 --input=input.txt --output=output.txt &>/dev/null; \
	cmp input.txt output.txt; \
	if [ $$? -ne 0 ]; \
	then \
	echo "Error with input file not having same text as output file"; \
	fi; \
	rm -f input.txt output.txt
