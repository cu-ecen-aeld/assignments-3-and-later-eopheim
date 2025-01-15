#!/bin/bash

writefile=$1

# Extract directory file filename path
writedir=$(dirname "$writefile")

writestr=$2

# Check if directory specified
if [ -z $writefile ]
then
	echo "Error: directory not specified"
	exit 1
fi

# Check if search string specified
if [ -z $writestr ]
then
	echo "Error: write string not specified"
	exit 1
fi

# Create directory if it doesnt already exist
if [ ! -d $writedir ] 
then
	mkdir -p $writedir
fi

# Write string to file
echo $writestr > $writefile

if [ ! -e $writefile ]
then
	echo "Error: unable to create file and or directory"
	exit 1
fi

