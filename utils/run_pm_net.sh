#! /bin/bash
if (( $# >= 1 ))
then	
	cp $1 $1_INIT
	FILE=$1_INIT
	FILE_ROOT=${FILE%.*}
	FILE_ROOT=${FILE_ROOT%@*}
	if (( $# > 1 ))
	then
		PRED_LR=$2
	else
		PRED_LR=0.001
	fi
	if (( $# > 2 ))
	then
		MIN_LR=$3
	else
		MIN_LR=0.0001
	fi
	for (( i=0; i<10; i++ ))
	do
		echo "FILE = "$FILE
		echo "minimize false" >> $FILE
		echo "learnRate ${PRED_LR}" >> $FILE		
		echo "momentum 0.0" >> $FILE
		rnn_lib -s $FILE
		FILE=`ls -rt ${FILE_ROOT}*best_sumSquaresError.save | tail -n 1`
		echo "FILE = "$FILE
		echo "minimize true" >> $FILE
		echo "learnRate ${MIN_LR}" >> $FILE		
		echo "momentum 0.9" >> $FILE
		rnn_lib -s $FILE
		FILE=`ls -rt ${FILE_ROOT}*best_sumSquaresError.save | tail -n 1`
	done
	cat ${FILE_ROOT}*.log > ${FILE_ROOT}.LOG
else
	echo "usage: run_pm_net.sh save_filename [pred_learn_rate min_learn_rate]"
fi
