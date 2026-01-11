#!/bin/bash

mount -t bpf bpf /sys/fs/bpf
rm -rf /sys/fs/bpf/*

bpftool prog load xdp_forward.o /sys/fs/bpf/xdp_forward type xdp
ip link set dev veth5_2 xdp off || echo "No xdp program to detach"
ip link set dev veth5_3 xdp off || echo "No xdp program to detach"
ip link set dev veth5_4 xdp off || echo "No xdp program to detach"
ip link set dev veth5_2 xdp pinned /sys/fs/bpf/xdp_forward
ip link set dev veth5_3 xdp pinned /sys/fs/bpf/xdp_forward
ip link set dev veth5_4 xdp pinned /sys/fs/bpf/xdp_forward
