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
echo "********** BEGIN TEST CASE 4 EASY v6 2n0r **********"
echo " "

# Sets pass and fail variables. zergmap will return 0 if the program
# processes all packets (good / bad). zergmap returns 1 if the program
# is unable to process the entirety of all .PCAP files. e.g. (malloc failed).
PASS="0"
FAIL="1"

# Passes all .pcap file names from all_pcap_files.txt ass command line
# arguments for zergmap and silences normal program output by outputting
# to dev/null.
./zergmap $(cat test/samp/test-case-4-easy_v6_2n0r.txt)

# Based upon the results, a pass or fail message is sent to stdout.
RESULTS="$?"
if [ "$RESULTS" == "$PASS" ]
then
    echo "Pass: zergmap test-case-4-easy_v6_2n0r."
else
    echo "FAIL: zergmap test-case-4-easy_v6_2n0r."
fi