#! /bin/bash
if (( $# > 0 ))
then	
	FILE=$1
	SIGFIGS=6
	PERT=1e-5
	VERBOSE=false
	HIDDENSIZE=1
	CODESIZE=1
if (( $# > 1 ))
then	
	HIDDENSIZE=$2
fi
if (( $# > 2 ))
then	
	SIGFIGS=$3
fi
if (( $# > 3 ))
then	
	PERT=$4
fi
if (( $# > 4 ))
then	
	VERBOSE=$5
fi
if (( $# > 5 ))
then	
	CODESIZE=$6
fi
	EXTRA_LEVELS=`grep hiddenSize $FILE | grep -o "," | wc -l | sed s/\ //g`
	#sed "s:hiddenSize:hiddenSize ${HIDDENSIZE}:g" $FILE |
	sed "s:codeSize:codeSize ${CODESIZE}:g" $FILE |
	sed "s:autosave:autosave false:g" |
	sed "s:sampling:sampling false:g" > GRAD_CHECK
	echo "gradCheck true" >> GRAD_CHECK
	echo "sigFigs ${SIGFIGS}" >> GRAD_CHECK
	echo "pert ${PERT}" >> GRAD_CHECK
	echo "verbose ${VERBOSE}" >> GRAD_CHECK
	echo -n "hiddenSize ${HIDDENSIZE}" >> GRAD_CHECK
	for ((l=0; l < $EXTRA_LEVELS ; l++)) 
		do echo -n ,${HIDDENSIZE} >> GRAD_CHECK
	done 
	rnnlib GRAD_CHECK
	rm GRAD_CHECK
else
        echo "usage: grad_test.sh filename [hiddenSize, sigFigs, pert, verbose]"
fi
