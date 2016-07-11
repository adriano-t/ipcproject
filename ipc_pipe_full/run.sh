#!/bin/bash



#==============================
# Compile
#==============================

if [[ $1 == "compile" || $1 == "c" ]]; then
	make clean
	make

elif [[ $1 == "compilerun" || $1 == "cr" ]]; then
	make clean
	make
	./ipc_calculator.x
elif [[ $1 == "free" || $1 == "ipcs" ]]; then

	IPCS_S=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | grep $(whoami) | cut -f2 -d" "`
	IPCS_M=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $(whoami) | cut -f2 -d" "`
	IPCS_Q=`ipcs -q | egrep "0x[0-9a-f]+ [0-9]+" | grep $(whoami) | cut -f2 -d" "`

	for id in $IPCS_M; do
	  ipcrm -m $id;
	done

	for id in $IPCS_S; do
	  ipcrm -s $id;
	done

	for id in $IPCS_Q; do
	  ipcrm -q $id;
	done
	
elif [[ $1 == "push" ]]; then
	git status
	git add --all
	git commit -m "Update: $2"
	git push
else
	./ipc_calculator.x
fi
 
