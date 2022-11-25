#!/bin/bash
make
nohup tcpdump -i ens4 -w project3.pcap &
TCP_PID=$!
echo "Launched TCPDUMP"
./server $1 $2
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out
make clean
echo "Exit Success"
echo ""