#!/bin/bash
PORT=$1
ip netns exec ns3 iptables -D INPUT -s 10.0.0.2 -p tcp --dport $PORT -j DROP
ip netns exec ns1 hping3 -I veth1_2 -c 1 -a 10.0.0.1 -p $PORT -s 124 10.0.0.3 > /dev/null || true
