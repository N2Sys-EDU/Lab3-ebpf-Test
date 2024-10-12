#!/bin/bash

ip netns exec ns3 iptables -F INPUT
ip netns exec ns3 iptables -P INPUT ACCEPT