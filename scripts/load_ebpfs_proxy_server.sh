#!/bin/bash
mount -t bpf bpf /sys/fs/bpf
rm -rf /sys/fs/bpf/*

bpftool prog load proxy_server_module_1.o /sys/fs/bpf/proxy_server_module_1 type xdp
ip link set dev veth4_5 xdp off || echo "No xdp program to detach"
ip link set dev veth4_5 xdp pinned /sys/fs/bpf/proxy_server_module_1