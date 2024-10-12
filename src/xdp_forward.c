#include "../common.h"

#include <linux/if_arp.h>

SEC("xdp_ingress")
int xdp_ingress_func(struct xdp_md* ctx) {
    void* data = (void*)(long)ctx->data;
    void* data_end = (void*)(long)ctx->data_end;
    struct ethhdr* eth = data;
    if ((void*)(eth + 1) > data_end) {
        return XDP_PASS;
    }
    if (eth->h_proto == __builtin_bswap16(ETH_P_IP)) {
        struct iphdr* ip = (struct iphdr*)(eth + 1);
        if ((void*)(ip + 1) > data_end) {
            return XDP_PASS;
        }
        struct bpf_fib_lookup fib = {};
        fib.family = AF_INET;
        fib.l4_protocol = ip->protocol;
        fib.ipv4_src = ip->saddr;
        fib.ipv4_dst = ip->daddr;
        fib.tot_len = __builtin_bswap16(ip->tot_len);
        fib.tos = ip->tos;
        fib.ifindex = ctx->ingress_ifindex;
        long status = bpf_fib_lookup(ctx, &fib, sizeof(fib), 0);
        if (status == 0) {
            __builtin_memcpy(eth->h_dest, fib.dmac, ETH_ALEN);
            __builtin_memcpy(eth->h_source, fib.smac, ETH_ALEN);
            return bpf_redirect(fib.ifindex, 0);
        }
        return XDP_PASS;
    }
    if (eth->h_proto == __builtin_bswap16(ETH_P_ARP)) {
        struct arphdr* arp = (struct arphdr*)(eth + 1);
        if ((void*)(arp + 1) > data_end) {
            return XDP_PASS;
        }
        if (arp->ar_hrd != __builtin_bswap16(ARPHRD_ETHER)) {
            return XDP_PASS;
        }
        if (arp->ar_pro != __builtin_bswap16(ETH_P_IP)) {
            return XDP_PASS;
        }
        if (arp->ar_hln != ETH_ALEN) { return XDP_PASS; }
        if (arp->ar_pln != 4) { return XDP_PASS; }
        if (arp->ar_op != __builtin_bswap16(ARPOP_REQUEST) && arp->ar_op != __builtin_bswap16(ARPOP_REPLY)) {
            return XDP_PASS;
        }
        if ((void*)(arp + 1) + 2 * ETH_ALEN + 2 * 4 > data_end) {
            return XDP_PASS;
        }

        __u32 src_ip = *(__u32*)((void*)(arp + 1) + ETH_ALEN);
        __u32 dst_ip = *(__u32*)((void*)(arp + 1) + ETH_ALEN * 2 + 4);

        struct bpf_fib_lookup fib = {};
        fib.family = AF_INET;
        fib.l4_protocol = IPPROTO_UDP;
        fib.ipv4_src = src_ip;
        fib.ipv4_dst = dst_ip;
        fib.tot_len = 28;
        fib.tos = 0;
        fib.ifindex = ctx->ingress_ifindex;
        long status = bpf_fib_lookup(ctx, &fib, sizeof(fib), 0);
        if (status == 0) {
            __builtin_memcpy(eth->h_dest, fib.dmac, ETH_ALEN);
            __builtin_memcpy(eth->h_source, fib.smac, ETH_ALEN);
            return bpf_redirect(fib.ifindex, 0);
        }
        return XDP_PASS;
    }

    return XDP_PASS;   
}

char _license[] SEC("license") = "GPL";