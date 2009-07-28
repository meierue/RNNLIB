if (( "$#" > "0" ));
then
	SCRIPT=normalise_inputs.py
	echo $#
	BASE=$1
	ADJ=${BASE}_ADJ
	echo $BASE
	echo $ADJ
	$SCRIPT $BASE $ADJ
	ncks -A $ADJ $1
	rm $ADJ
	while [ $# -gt 1 ]
		do
		shift
		ORIG=$1
		ADJ=${ORIG}_ADJ
		echo $ORIG
		echo $ADJ
		$SCRIPT -s $BASE $ORIG $ADJ
		ncks -A $ADJ $ORIG
		rm $ADJ
		done
else
	echo "usage: adj_std_dev_mean.sh nc_file_1 [nc_file_2, nc_file_3,...]"
fi

