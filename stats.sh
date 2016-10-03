clear

argumentCount="$#"
transposeFile="transposeFile"
#tempRow="./tempRow"
inputFile="dataFile$$"
#tempCol="./tempCol"


if [ "$argumentCount" = "1" ] #checks if there was one argument passed.
then
	cat > "$inputFile" #adds the stdin data to be stored in datafile(process ID).
elif [ "$argumentCount" = "2" ]  #checks if there were two arguments passed.
then
	if [ -s $2 ] #checks if files exists and has contents.
	then
		inputFile=$2 #stores the input filepath argumment.
	else 
		echo "ERROR: File not readable." 1>&2 # write error message to
		exit 1 #standard error and exit with value of 1.
	fi
else 
	echo "ERROR: Invalid number of arguments." 1>&2 #write error message to 
	exit 1 #standard error and return 1 for too many arguments passed.
fi

trap "rm -rf ./dataFile$$ || true; echo 'Trapped a signal... Exiting.' exit 1;" IN HUP TERM #traps and terminates file storing stdin data if created.

if [[ $1 == -r* ]] #checks if the user wants the data sorted by rows.
then
	echo "calculating row stats."
	while read i #while not the end of the file, read the next row.
	do
		median=0 #set median equal to 0
		count=0 #set row number count to 0 for average.
		sum=0 #set sum to 0 to be divided by count for average.
		for num in $i #for each number in each row i.
		do	
			numArray[count]=$(($num))
			sum=$(($sum + $num)) #sum the rows
			count=$(($count + 1)) #increment count
		done
		sortedNumArray=($(for j in "${numArray[@]}"; do echo -e $j; done | sort -n)) #interates through the array of row values, sorts them, and echoes them out into a new sorted array.
		average=$(($sum / $count)) #at end of row, calculate average
		for enum in "${sortedNumArray[@]}"
		do
			echo "$enum"
		done | sort -n
		median=${sortedNumArray[$(($count/2))]}
		echo -e "Average: $average Median: $median" #print average and median for each row
	done < "$inputFile"
elif [[ $1 == -c* ]] #checks if the users wants the data sorted by columns.
then
	tempCol="./tempCol$$" #create temp files for column and row for transposing
	tempRow="./tempRow$$"
	finalRow="./finalRow$$"
	echo "calculating column stats."
	count=0 #holds the number of values per row at the end of loop
	while read i
	do 	
		cut -f $(($count+1)) $inputFile | sort >> "$tempCol" #cuts the columns into tempCol and sorts them.
		count=$(($count+1)) #increment counter.
	done < $"$inputFile" #sends in the inputfile to the loop.
	cat $tempCol | tr '\n' '\t' > $tempRow #replaces newline chracters with tab characters to reformat into rows.
	transposecounter=$(($count*2))
	sed -e "s/.\{$transposecounter\}/&\n/g" < $tempRow | cat >> "$finalRow"

	while read line 
	do 	
		count=0
		median=0
		average=0
		sum=0
		for enum in $line 
		do 
			sum=$(( $sum+$enum ))
			sortedNumArray[count]=$enum
			count=$(( $count+1 ))
		done
		average=$(($sum / $count))
		median=${sortedNumArray[$(($count/2))]}
		echo "Average: $average Median: $median"

	done < "$finalRow"
	rm -f $tempCol
	rm -f $tempRow
	#rm -f $finalRow
else 
	echo "ERROR: You did not validly ask for a row or column calculation." 1>&2
	exit 1
fi


