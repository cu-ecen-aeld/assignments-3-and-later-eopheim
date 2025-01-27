#!/bin/sh

filesdir=$1

searchstr=$2

# Check if directory specified
if [ -z $filesdir ]
then
	echo "Error: directory not specified"
	exit 1
fi

# Check if search string specified
if [ -z $searchstr ]
then
	echo "Error: search string not specified"
	exit 1
fi

# Check if file specified exists
if [ ! -d $filesdir ]
then
	echo "Error: directory does not exist"
	exit 1
fi

# Variables for saving line and file counts
num_lines=0
num_files=0

# Loop through each file in specified directory
for file in "$filesdir"/*
do
	# Check file is not a directory
	if [ ! -d $file ]
	then
		# Save total line matches
		num_lines=$((num_lines+$(grep -c $searchstr $file)))
		
		# Increment total file match count
		num_files=$((num_files+1))
	fi
done

echo "The number of files are $num_files and the number of matching lines are $num_lines."

