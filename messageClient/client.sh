#!/bin/bash
nohup tcpdump -i ens4 dst $1 -w project1.pcap -c 20 &
TCP_PID=$!
echo "Launched TCPDUMP"
./program $1 $2 $3
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out