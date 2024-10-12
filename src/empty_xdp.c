#include "common.h"

SEC("xdp_ingress")
int xdp_ingress_func(struct xdp_md* ctx) {
	return XDP_PASS;
}
