#! /bin/bash
if (( $# > 0 ))
then	
	SAVE_FILE=$1	
	if (( $# == 1))
	then
		SAVE_STRIP=${SAVE_FILE##*\/} 		
		LOG_FILE=err_test_${SAVE_STRIP%.*}.log
	else
		LOG_FILE=$2
	fi
	cp $SAVE_FILE ERR_TEST_TEMP 
	echo "errorTest true" >> ERR_TEST_TEMP
	echo "autosave false" >> ERR_TEST_TEMP
	echo "verbose true" >> ERR_TEST_TEMP
	rnnlib ERR_TEST_TEMP | tee $LOG_FILE
	rm ERR_TEST_TEMP
else
        echo "usage: err_test_boost.sh save_filename [log_filename]"
fi
