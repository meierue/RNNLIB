#! /bin/bash
if (( $# > 1 ))
then	
	SAVE_FILE=$1	
	DICT_FILE=$2
	if (( $# == 2))
	then
		SAVE_STRIP=${SAVE_FILE##*\/} 		
		LOG_FILE=dict_err_test_${SAVE_STRIP%.*}_${DICT_FILE##*\/}.log
	else
		LOG_FILE=$3
	fi
	cp $SAVE_FILE DICT_ERR_TEST_TEMP 
	echo "errorTest true" >> DICT_ERR_TEST_TEMP
	echo "autosave false" >> DICT_ERR_TEST_TEMP
	echo "verbose true" >> DICT_ERR_TEST_TEMP
	echo "dictionary "${DICT_FILE} >> DICT_ERR_TEST_TEMP
	rnnlib DICT_ERR_TEST_TEMP | tee $LOG_FILE
	rm DICT_ERR_TEST_TEMP
else
        echo "usage: dict_err_test_boost.sh save_filename dict_filename [log_filename]"
fi
