clear

datafilepath="datafile$$"
if [ "$#" = "1" ] #checks if there was one  argument passed.
then
	cat > "$datafilepath" #adds the stdin data to new file.
elif [ "$#" = "2" ]  #checks if there were two arguments passed.
then
	datafilepath=$2 #stores the data filepath argument.
fi

if [[ $1 == -r* ]] #checks if the user wants the data sorted by rows.
then
	echo "calculating row stats."
	while read i #while not the end of the file.
	do
		median=0
		medianIndex=0
		count=0 #set row number count to 0 for average.
		sum=0 #set sum to 0 to be divided by count for average.
		for num in $i #for each number in each row i.
		do	
			numArray[count]=$num
			sum=$(($sum + $num)) #sum the rows
			count=$(($count + 1)) #increment count
		done
		sortedNumArray=($(for j in "${numArray[@]}"; do echo $j; done | sort -n))
		average=$(($sum / $count)) #at end of row, calculate average
		for enum in "${numArray[@]}"
		do
			echo "$enum"
		done | sort
		if [ $(($count%2)) -eq 0 ]
		then 
			medianIndex=$(($count/2 + 1))
		else
			medianIndex=$(($count/2))
		fi
		median=${sortedNumArray[$medianIndex]}
		echo -e "Average: $average Median: $median Median Index: $medianIndex" #print average and median for each row
	done < $2
elif [[ $1 == -c* ]] #checks if the users wants the data sorted by columns.
then
	echo "calculating column stats." 
fi

