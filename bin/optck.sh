#!/bin/sh
#
# Check whether Assembler optimization is correct or not.
#

if [ -f ./optck ]; then
	/bin/rm -f ./optck
fi
/bin/cc -O2 ../src/optck.c -o optck
res=`./optck`

if [ "$res" = "wrong" ]; then
	echo -n "-Qoption as -O~M"
fi



