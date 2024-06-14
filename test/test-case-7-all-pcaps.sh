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
echo " "
echo "********** BEGIN AUTOMATED INTEGRATION TESTING **********"
echo " "

# Sets pass and fail variables. zergmap will return 0 if the program
# processes all packets (good / bad). zergmap returns 1 if the program
# is unable to process the entirety of all .PCAP files. e.g. (malloc failed).
PASS="0"
FAIL="1"

# Searches for all .pcap files in the directory and subdirectories of
# test/samp. zergmap then executes for every given .pcap file.
find test/samp -type f -name "*.pcap" | while read fname
do
    # Output to dev/null silences normal program output.
    ./zergmap ${fname} > /dev/null 2>&1;

    # Assigns the value of the exist status of the last command executed ($?)
    # to RESULTS variable.
    RESULTS="$?"

    # Compares results string against pass / fail variable strings.
    if [ "$RESULTS" == "$PASS" ]
    then
        echo "Pass: ${fname}."
    else
        echo "Fail: ${fname}."
    fi
done

# Passes all .pcap file names from all_pcap_files.txt ass command line
# arguments for zergmap and silences normal program output by outputting
# to dev/null.
./zergmap -h 99 $(cat test/samp/all_pcap_files.txt) > /dev/null 2>&1

# Based upon the results, a pass or fail message is sent to stdout.
RESULTS="$?"
if [ "$RESULTS" == "$PASS" ]
then
    echo "Pass: zergmap all_pcap_files.txt."
else
    echo "FAIL: zergmap all_pcap_files.txt."
fi