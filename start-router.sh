#!/bin/bash

#Assumed first router name as ‘A’ and port no. 2000
#Executed Other router name in alphabetical order
#Assigned port no. sequentially
#To assign different router name and port no. change in start-router1
#Run From Terminal ./start-router or ./start-router1



first1=config
second1=.txt
first2=routing-output
second2=.txt
char=A
inputfilename=configA.txt
port=2000

for ((i=0;i<26;i++)) ;
do	
	if [ -f "$inputfilename" ] ;
	then xterm -title "$char" -hold -e ./dv_routing "$char" "$port" "$inputfilename" & sleep 1
	char=$(echo "$char" | tr "0-9A-Z" "1-9A-Z_")
	inputfilename=${first1}${char}${second1}
	port=$((port+1))
	fi;
done;
