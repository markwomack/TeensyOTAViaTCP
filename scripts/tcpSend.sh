#!/usr/bin/bash

# This script will take a copy of a file, prepend the
# original file size and crc to it, and then send the
# modified file contents over TCP to a target ip address
# and port.
#
# Example of usage:
#   ./tcpSend.sh 192.168.94.10 50000 somefilename
#                ------------- ----- ------------
#                      ^         ^        ^ file contents to send over tcp
#                      |         | target port to send the content to
#                      | target ip address to send the conent to

# modified file that will be sent
f=$(echo $3.tcp)

# copy the original file to/over the modified file
cp $3 $f

# get size of file
s=$(echo $(stat -c "%s" $3)$(echo "!"))

# get crc of file
crc=$(echo $(crc32 $3)$(echo "!"))

# prepend the size and crc to the file
sed -i "1s/^/$s$crc/" $f

# send the modified file to the target ip and port
netcat -N $1 $2 <$f
