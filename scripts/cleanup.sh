#!/bin/bash
for i in {1..5}; do
    ip netns del ns$i
done

ip link del veth1_2
ip link del veth2_5
ip link del veth3_5
ip link del veth4_5

ip link del br0

userdel lab3_user

echo "Network namespaces, virtual Ethernet interfaces, and bridge interface have been cleaned up."