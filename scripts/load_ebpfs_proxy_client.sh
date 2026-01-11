#!/bin/bash
mount -t bpf bpf /sys/fs/bpf
rm -rf /sys/fs/bpf/*
bpftool prog load proxy_client_module_2.o /sys/fs/bpf/proxy_client_module_2 type xdp
ip link set dev veth1_2 xdp off || echo "No xdp program to detach"
ip link set dev veth1_2 xdp pinned /sys/fs/bpf/proxy_client_module_2