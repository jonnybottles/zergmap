#! /bin/bash

# Bash Scripting References:
# https://www.tutorialkart.com/bash-shell-scripting/bash-else-if/
# https://linuxize.com/post/how-to-compare-strings-in-bash/
# https://linuxhint.com/bash_command_output_variable/
# https://stackoverflow.com/questions/38480480/bash-execute-a-command-on-all-
# files-with-extension-recursively

# Changes directory to parent directory where zergmap is stored to
# enable to execution of zergmap
cd ../

# Sets pass and fail variables. 
PASS="0"
FAIL="1"

echo " "
echo "********** BEGIN VALGRIND TESTING **********"
echo " "


# Searches for all .pcap files in the directory and subdirectories of
# test/samp. zergmap then executes for every given .pcap file with
# valgrind / zergmap.
find test/samp -type f -name "*.pcap" | while read fname
do
    # Output to dev/null silences normal program output.
    # Valgrind error code ref: https://stackoverflow.com/questions/19246470/
    #how-to-get-in-script-whether-valgrind-found-memory-leaks
    valgrind --error-exitcode=1 -s --leak-check=full --track-origins=yes ./zergmap ${fname} > /dev/null 2>&1;

    # Assigns the value of the exist status of the last command executed ($?)
    # to RESULTS variable.
    VALGRIND_RESULTS="$?"

    # Compares results string against pass / fail variable strings.
    if [ "$VALGRIND_RESULTS" == "$PASS" ]
    then
        echo "Pass valgrind: ${fname}."
    else
        echo "FAIL valgrind: ${fname}."
    fi
done


# Passes all .pcap file names from all_pcap_files.txt ass command line
# arguments for zergmap and silences normal program output by outputting
# to dev/null.
valgrind --error-exitcode=1 -s --leak-check=full --track-origins=yes ./zergmap -h 99 $(cat test/samp/all_pcap_files.txt) > /dev/null 2>&1

# Based upon the results, a pass or fail message is sent to stdout.
    VALGRIND_RESULTS="$?"
    # Compares results string against pass / fail variable strings.
    if [ "$VALGRIND_RESULTS" == "$PASS" ]
    then
        echo "Pass valgrind: all_pcap_files.txt."
    else
        echo "FAIL valgrind: all_pcap_files.txt."
    fi

# Single valgrind test against impropr file extension.
valgrind --error-exitcode=1 -s --leak-check=full --track-origins=yes ./zergmap file.txt > /dev/null 2>&1;
    VALGRIND_RESULTS="$?"
    # Compares results string against pass / fail variable strings.
    if [ "$VALGRIND_RESULTS" == "$PASS" ]
    then
        echo "Pass valgrind: improper file extension"
    else
        echo "FAIL valgrind: improper file extension"
    fi

# Single valgrind test against non-existent file.
valgrind --error-exitcode=1 -s --leak-check=full --track-origins=yes ./zergmap non_existent.pcap > /dev/null 2>&1;
    VALGRIND_RESULTS="$?"
    # Compares results string against pass / fail variable strings.
    if [ "$VALGRIND_RESULTS" == "$PASS" ]
    then
        echo "Pass valgrind: non_existent.pcap"
    else
        echo "FAIL valgrind: non_existent.pcap"
    fi

# Single valgrind test against testrun.c
valgrind --error-exitcode=1 -s --leak-check=full --track-origins=yes ./test/test-run > /dev/null 2>&1;
    VALGRIND_RESULTS="$?"
    # Compares results string against pass / fail variable strings.
    if [ "$VALGRIND_RESULTS" == "$PASS" ]
    then
        echo "Pass valgrind: test-run"
    else
        echo "FAIL valgrind: test-run"
    fi


echo " "
echo "********** BEGIN AUTOMATED UNIT TESTING **********"
echo " "