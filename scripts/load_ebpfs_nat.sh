#!/bin/bash
mount -t bpf bpf /sys/fs/bpf
rm -rf /sys/fs/bpf/*

bpftool prog load nat_module_1.o /sys/fs/bpf/nat_module_1 type classifier
bpftool prog load nat_module_2.o /sys/fs/bpf/nat_module_2 type xdp
bpftool net detach xdp dev veth2_5 || echo "No xdp program to detach"
tc qdisc del dev veth2_5 clsact || echo "No qdisc to delete"
tc qdisc add dev veth2_5 clsact
tc filter add dev veth2_5 egress bpf da object-pinned /sys/fs/bpf/nat_module_1 sec tc_egress
bpftool net attach xdp pinned /sys/fs/bpf/nat_module_2 dev veth2_5

bpftool prog load proxy_client_module_1.o /sys/fs/bpf/proxy_client_module_1 type xdp
bpftool net detach xdp dev veth2_1 || echo "No xdp program to detach"
bpftool net attach xdp pinned /sys/fs/bpf/proxy_client_module_1 dev veth2_1
