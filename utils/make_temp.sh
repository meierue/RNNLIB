#! /bin/bash
if [ $# = 2 ]
then
	F=$1
	sed "s:autosave:autosave false:g" $F |
	sed "s:trainFrac:trainFrac ${2}:g" > TEMP
else
        echo "usage: make_temp.sh filename train_frac"
fi
