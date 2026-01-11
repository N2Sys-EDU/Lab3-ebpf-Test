#!/bin/bash

NS=$1
shift
PORT="$*"

rm -f ./.server_started
echo "Starting server in network namespace $NS on port $PORT..."
nohup ip netns exec $NS ./lab3_server $PORT &

while [ ! -f ./.server_started ]; do
    sleep 1
done
echo "Server started."
