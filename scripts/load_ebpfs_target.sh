#!/bin/bash

mount -t bpf bpf /sys/fs/bpf
rm -rf /sys/fs/bpf/*

bpftool prog load xdp_empty.o /sys/fs/bpf/xdp_empty type xdp
ip link set dev veth3_5 xdp off || echo "No xdp program to detach"
ip link set dev veth3_5 xdp pinned /sys/fs/bpf/xdp_empty
