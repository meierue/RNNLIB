#!/bin/bash
if [ $# = 4 ]
then
	D=jacobian_$1_$2_$3_$4/
	F=JACOBIAN_TMP
	mkdir $D
	cp $1 $F
	echo dataset $2 >> $F
	echo sequence $3 >> $F
	echo jacobianCoords $4 >> $F
	echo displayPath $D >> $F
	echo autosave false >> $F
	echo verbose true >> $F
	rnn_lib $F
	rm $F
else
	echo "usage: jacobian_boost.sh net_save dataset[train|test|val] seq_number jacobian_coords"
fi
