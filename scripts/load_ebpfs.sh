#!/bin/bash

clang-15 -O2 -target bpf -c ../user/nat_module_1.c -o nat_module_1.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g
clang-15 -O2 -target bpf -c ../user/nat_module_2.c -o nat_module_2.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g
clang-15 -O2 -target bpf -c ../user/proxy_client_module_1.c -o proxy_client_module_1.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g
clang-15 -O2 -target bpf -c ../user/proxy_client_module_2.c -o proxy_client_module_2.o -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g
clang-15 -O2 -target bpf -c ../user/proxy_server_module_1.c -o proxy_server_module_1.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g

clang-15 -O2 -target bpf -c ../test_utils/src/xdp_empty.c -o xdp_empty.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g
clang-15 -O2 -target bpf -c ../test_utils/src/xdp_forward.c -o xdp_forward.o  -I/usr/include/x86_64-linux-gnu -D__x86_64__ -g

ip netns exec ns5 bash ../test_utils/scripts/load_ebpfs_router.sh
ip netns exec ns3 bash ../test_utils/scripts/load_ebpfs_target.sh
ip netns exec ns2 bash ../test_utils/scripts/load_ebpfs_nat.sh
ip netns exec ns4 bash ../test_utils/scripts/load_ebpfs_proxy_server.sh
ip netns exec ns1 bash ../test_utils/scripts/load_ebpfs_proxy_client.sh
