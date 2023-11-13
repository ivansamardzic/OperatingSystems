#!/bin/bash

# ELEC377 - Operating System
# Lab 4 - Shell Scripting, ps.sh

# Program Description: Shell script to list running processes.

# Variables to store shell commands from user.
showRSS="no"
showComm="no"
showCommand="no"
showGroup="no"

# Phase 1 - Parse arguments inputted to the command line.
# Loop while current argument not empty. 
while [ -n "$1" ]; do
    # Check if current argument matches any of the recognized flags.
    # Set flag variable to yes if a match is found.
    if [ "$1" == "-rss" ]; then
        showRSS="yes"
    elif [ "$1" == "-group" ]; then
        showGroup="yes"
    elif [ "$1" == "-comm" ]; then
        if [ "$showCommand" == "yes" ]; then
            # Error case for input of both comm and command flags.
            echo "Error: Can only specify one of -comm and -command flags."
            exit 1
        fi
        showComm="yes"
    elif [ "$1" == "-command" ]; then
        if [ "$showComm" == "yes" ]; then
            # Error case for input of both comm and command flags.
            echo "Error: Can only specify one of -comm and -command flags."
            exit 1
        fi
        showCommand="yes"
    else
        echo "Error: Flag '$1' not recognized."
        exit 1
    fi
    shift
done
# Phase 4 - Creating a temporary file to hold the output write.
tmpFile="/tmp/tmpPs$$.txt"


# Phase 2 - Identify /proc path for processes. 
# Iterate through/proc directories 0-9, corresponding to respective processes. 
for p in /proc/[0-9]*; do
    # Check if current directory still exists.
    if [ -d "$p" ]; then
        # Phase 3 - Extract information of current process. 
        # Extract process ID, command name, numeric user ID, rss, and numeric group ID from status file.
        # Use grep/sed commands.

        # Extracting the process ID.
        pid=`grep '^Pid' $p/status | sed -e 's/Pid:\s*//'`

        # Extracting the command name.
        cmd=`grep '^Name' $p/status | sed -e 's/Name:\s*//'`

        # Extracting the numeric user ID.
        # Phase 3b - Covert the numeric uid to its symbolic name.
        # Use awk command to search /etc/passwd.
        uidNumeric=`grep '^Uid' $p/status | sed -e 's/Uid:\s*//' | awk -F'[^0-9]+' '{print $2}'`
        uid=`awk -F':' -v uidNumeric="$uidNumeric" '{if ($3 == uidNumeric) print $1}' /etc/passwd`


        # Extracting the rss value.
        # Remove any non-numeric characters (kB) from the value.
        # Check if rss value exists. If not, set to zero.
        rss=`grep '^VmRSS' $p/status | sed -e 's/VmRSS:\s*//' | awk -F'[^0-9]+' '{print $1}'`
        if [ -z "$rss" ]; then
            rss=0
        fi

        # Extracting the numeric group ID.
        # Phase 3b - Covert the numeric gid to its symbolic name.
        # Use awk command to search /etc/group.
        gidNumeric=`grep '^Gid' $p/status | sed -e 's/Gid:\s*//' | awk -F'[^0-9]+' '{print $2}'`
        gid=`awk -F':' -v gidNumeric="$gidNumeric" '{if ($3 == gidNumeric) print $1}' /etc/group`
 

        # Phase 4 - Print output to file, ensuring data is neatly aligned.
        # Only print a given category if specified by the user. Check status of shell commands like before to determine this.
        # Each new line is appended.
        # Always print pid and uid, no matter the user input.
        printf "%-20s" $pid >> $tmpFile
        printf "%-20s" $uid >> $tmpFile
        if [[ $showGroup == "yes" ]]; then
            printf "%-20s" $gid >> $tmpFile
        fi
        if [[ $showRSS == "yes" ]]; then
            printf "%-20s" $rss >> $tmpFile
        fi   
        if [[ $showComm == "yes" || $showCommand == "yes" ]]; then
            printf "%-20s" $cmd >> $tmpFile
        fi
        # Print a new line once all neccessary values for the current iteration printed.
        printf "\n" >> $tmpFile
    fi
done

# Phase 5 - Printing the header to the terminal.
# Only print headings if specified by the user. Check status of shell commands like before to determine this.
# Always print PID and User, no matter the user input.
printf "%-20s" "PID"
printf "%-20s" "User"
if [[ $showGroup == "yes" ]]; then
    printf "%-20s" "Group"
fi
if [[ $showRSS == "yes" ]]; then
    printf "%-20s" "RSS"
fi
if [[ $showComm == "yes" || $showCommand == "yes" ]]; then
    printf "%-20s" "Command"
fi
# Print a new line once all neccessary headings printed.    
printf "\n"


# Sort the file by its first column, PID.
sort -n -k1 $tmpFile
# Remove temporary file upon completion.
rm -f $tmpFile