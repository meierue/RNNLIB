#!/bin/bash
if (( $# >= 3 ))
then
	D=display_$1_$2_$3
	if (( $# > 3 ))
	then
		D=${D}_${4}
	fi
	D=${D}/
	F=DISPLAY_TMP
	mkdir $D
	cp $1 $F
	echo display true >> $F
	echo displayPath $D >> $F
	if (( $# == 3 ))
	then
		echo sequence $3 >> $F
	else
		echo dataFileNum $3 >> $F
		echo sequence $4 >> $F
	fi
	echo autosave false >> $F
	echo verbose true >> $F
	echo dataset $2 >> $F
	rnnlib $F
	rm $F
else
	echo "usage: dump_sequence_variables.sh net_save dataset(train|test|val) [data_file_number] seq_number"
fi
