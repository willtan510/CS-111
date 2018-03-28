#!/bin/bash

#Incorrect option test
echo | ./lab4b --wrong &>/dev/null; \
if [[ $? -ne 1 ]]; \
then \
	echo "Invalid option that was not caught by program"; \
fi
	
#Multiple inputs test
./lab4b --period=5 --scale=C --log=logging <<-EOF
PERIOD=2
STOP
SCALE=F
START
OFF	
EOF
ret=$?
if [ $ret -ne 0 ]
then 
	echo "Inaccurate exit for corresponding valid arguments and commands"
else
  echo "Passed Input test"
fi

#Log File Creation test
if [ ! -s logging ]
then
	echo "Logging file failed to be created"
else
  echo "Passed File Creation Test"
fi

#Logged inputs and temperature reports
grep "PERIOD=2" logging &> /dev/null; \
if [ $? -ne 0 ]
then
	echo "PERIOD= command not found in log file"
else
  echo "Passed PERIOD= test"
fi

grep "STOP" logging &> /dev/null; \
if [ $? -ne 0 ]
then
	echo "STOP command not found in log file"
else
  echo "Passed STOP test"
fi

grep "SCALE=F" logging &> /dev/null; \
if [ $? -ne 0 ]
then
	echo "SCALE=F command not found in log file"
else
  echo "Passed SCALE=F test"
fi

grep "START" logging &> /dev/null; \
if [ $? -ne 0 ]
then 
	echo "START command not found in log file"
else
  echo "Passed START test"
fi

grep "SHUTDOWN" logging &> /dev/null; \
if [ $? -ne 0 ]
then
	echo "SHUTDOWN command not found in log file"
else
  echo "Passed SHUTDOWN test"
fi

egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9][0-9].[0-9]' logging &> /dev/null; \
if [ $? -ne 0 ]
then
	echo "Date and Temperature not properly outputted"
else
  echo "Passed Date & Temperature Test"
fi
