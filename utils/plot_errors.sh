#! /bin/bash

UTILS=/home/alex/code/neural_net_console-1.0/utils
PL=plot_errors.py
if [ $# = 4 ]
then
	FILE=$1_$2_$3_$4
	grep -A $4 "$2 errors" $1 | grep $3 | cut -f2 -d" " > $FILE
	$PL $FILE
#	rm $FILE
else
	echo "usage: plt_errors.sh save_file data_set(train|test|validation) error_type(labelErrorRate,ctcMlError...) num_error_types"
fi
