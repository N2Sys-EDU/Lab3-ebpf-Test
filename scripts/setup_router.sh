sysctl -w net.ipv4.ip_forward=1
ip link set veth5_2 up
ip link set veth5_3 up
ip link set veth5_4 up
ip link set lo up
ip route add 10.0.0.2 dev veth5_2
ip route add 10.0.0.3 dev veth5_3
ip route add 10.0.0.4 dev veth5_4
echo 1 | tee /proc/sys/net/ipv4/conf/veth5_2/proxy_arp
echo 1 | tee /proc/sys/net/ipv4/conf/veth5_3/proxy_arp
echo 1 | tee /proc/sys/net/ipv4/conf/veth5_4/proxy_arp

ping -c 1 10.0.0.2 -W 0.1s
ping -c 1 10.0.0.3 -W 0.1s
ping -c 1 10.0.0.4 -W 0.1s