#!/bin/bash
if [ -n "$3" ]; then
make
nohup tcpdump -i ens4 -w project2.pcap &
TCP_PID=$!
echo "Launched TCPDUMP"
./server $1 $2 $3
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out
make clean
echo "Exit Success"
echo ""
else
make
nohup tcpdump -i ens4 -w project2.pcap &
TCP_PID=$!
echo "Launched TCPDUMP"
./server $1 $2
kill $TCP_PID
echo "Killed TCPDUMP"
rm nohup.out
make clean
echo "Exit Success"
echo ""
fi