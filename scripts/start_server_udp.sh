#!/bin/bash

NS=$1
shift
PORT="$*"

rm -f ./.server_udp_started
echo "Starting server in network namespace $NS on port $PORT..."
nohup ip netns exec $NS ./lab3_server_udp $PORT &

while [ ! -f ./.server_udp_started ]; do
    sleep 1
done
echo "UDP Server started."
