#! /bin/bash
if [ $# = 2 ]
then
	grep $1 $2 | cut -d ' ' -f 2 | python /home/alex/std_dev_mean.py
else
	echo "usage: search_pattern file_pattern"
fi

