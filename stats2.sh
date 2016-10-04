clear

transposeFile="transposeFile"
tempRow="./tempRow"
dataFilePath="datafile$$"
tempCol="./tempCol"


if [ "$#" = "1" ] #checks if there was one  argument passed.
then
	cat > "$dataFilePath" #adds the stdin data to new file.
elif [ "$#" = "2" ]  #checks if there were two arguments passed.
then
	dataFilePath=$2 #stores the data filepath argument.
fi

if [[ $1 == -r* ]] #checks if the user wants the data sorted by rows.
then
	echo "calculating row stats."
	while read i #while not the end of the file.
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
		median=${sortedNumArray[$(($count/2 ))]}
		echo -e "Average: $average Median: $median" #print average and median for each row
	done < "$dataFilePath"
elif [[ $1 == -c* ]] #checks if the users wants the data sorted by columns.
then
	echo "calculating column stats."
	count=0
	while read i
	do 		
		if [ $count -eq 0 ]
		then
			cut -f $(($count+1)) $dataFilePath > "$tempCol"
		else 
			cut -f $(($count+1)) $dataFilePath >> "$tempCol"
		fi
		count=$(($count+1))  
		cat $tempCol | tr '\n' '\t' > $tempRow
		echo -e "" >> $tempRow
		while read line
		do 
			median=0	
			sum=0
			count2=0
			for enum in $line
			do	
				numArray[count2]=$(($enum))
				sum=$(($sum + $enum))
				count2=$(($count2 + 1)) 
			done
			sortedNumArray=($(for k in "${numArray[@]}"; do echo -e $k; done | sort -n))
			average=$(($sum/$count2))
			for sortedNumbers in "${sortedNumArray[@]}"
			do
				echo $sortedNumbers
			done | sort -n
			median=${sortedNumArray[$(($count2/2))]}
			echo -e "Average: $average Median: $median"
		done < "$tempRow"
		rm $tempRow
		rm $tempCol
	done < "$dataFilePath"
fi

