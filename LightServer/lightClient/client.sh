#!/bin/bash
make
nohup tcpdump -i ens4 dst $1 -w project3.pcap -c 20 &
TCP_PID=$!
echo "Launched TCPDUMP"
./switch $1 $2 $3
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out
make clean
echo "Exit Success"
echo ""