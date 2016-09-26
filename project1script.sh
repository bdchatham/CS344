clear

datafilepath="datafile$$"
if [ "$#" -eq "1" ] #checks if there was no argument passed.
then
	cat > "$datafilepath" #adds the stdin data to the file.
elif [ "$#" -eq "2" ]  #checks if there was a filepath argument passed.
then
	datafilepath=$2 #stores the data filepath argument.
fi

if [[ $1 == -r* ]] #checks if the user wants the data sorted by rows.
then
	echo "calculating row stats."
elif [[ $1 == -c* ]] #checks if the users wants the data sorted by columns.
then
	echo "calculating column stats." 
fi

 
