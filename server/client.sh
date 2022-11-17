#!/bin/bash
nohup tcpdump -i ens4 dst -w project1.pcap -c 20 &
TCP_PID=$!
echo "Launched TCPDUMP"
./server $1 $2
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out
echo "Exit Success"