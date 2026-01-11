#!/bin/bash

# 创建命名空间
for i in {1..5}; do
    ip netns add ns$i
done

ip link add veth1_2 type veth peer name veth2_1
ip link add veth2_5 type veth peer name veth5_2
ip link add veth3_5 type veth peer name veth5_3
ip link add veth4_5 type veth peer name veth5_4
ip link set veth5_2 promisc on
ip link set veth5_3 promisc on
ip link set veth5_4 promisc on

# ns1
ip link set veth1_2 netns ns1
ip netns exec ns1 ethtool -K veth1_2 rx off tx off
ip netns exec ns1 ip addr add 192.168.1.2/24 dev veth1_2
ip netns exec ns1 ip link set veth1_2 up
ip netns exec ns1 ip link set lo up
ip netns exec ns1 ip route add default via 192.168.1.1

# ns2
ip link set veth2_1 netns ns2
ip link set veth2_5 netns ns2
ip netns exec ns2 ethtool -K veth2_1 rx off tx off
ip netns exec ns2 ethtool -K veth2_5 rx off tx off
ip netns exec ns2 sysctl -w net.ipv4.ip_forward=1
ip netns exec ns2 ip addr add 192.168.1.1/24 dev veth2_1
ip netns exec ns2 ip link set veth2_1 up
ip netns exec ns2 ip addr add 10.0.0.2/8 dev veth2_5
ip netns exec ns2 ip link set veth2_5 up
ip netns exec ns2 ip link set lo up
ip netns exec ns2 ip route add default via 10.0.0.1

# ns3
ip link set dev veth3_5 address 02:a2:a3:a4:a5:a6
ip link set veth3_5 netns ns3
ip netns exec ns3 ethtool -K veth3_5 rx off tx off
ip netns exec ns3 ip addr add 10.0.0.3/8 dev veth3_5
ip netns exec ns3 ip link set veth3_5 up
ip netns exec ns3 ip link set lo up
ip netns exec ns3 ip route add default via 10.0.0.1

# ns4
ip link set veth4_5 netns ns4
ip netns exec ns4 ethtool -K veth4_5 rx off tx off
ip netns exec ns4 sysctl -w net.ipv4.ip_forward=1
ip netns exec ns4 ip addr add 10.0.0.4/8 dev veth4_5
ip netns exec ns4 ip link set veth4_5 up
ip netns exec ns4 ip link set lo up
ip netns exec ns4 ip route add default via 10.0.0.1

# ns5
ip link set veth5_2 netns ns5
ip link set veth5_3 netns ns5
ip link set veth5_4 netns ns5
ip netns exec ns5 ./../test_utils/scripts/setup_router.sh

useradd lab3_user
echo "Setup done!"