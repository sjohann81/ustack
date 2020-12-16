/* file:          ip.c
 * description:   IPv4 protocol implementation (RFC791)
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdio.h>
#include <string.h>
#include "include/ustack.h"


static int32_t (*ip_callback)(uint8_t *packet);

int32_t ip_out(uint8_t dst_addr[4], int16_t proto, uint8_t *packet, uint16_t len)
{
	uint16_t sum;
	int32_t val;
	struct ip_s *ip = (struct ip_s *)packet;

	ip->ver = IP_VER_IHL >> 8;
	ip->tos = 0x00;
	ip->len = htons(len);
	ip->id = htons(0);
	ip->flags_off = htons(0);
	ip->ttl = IP_DEFAULT_TTL;
	if (proto > -1)
		ip->proto = proto & 0xff;
	memcpy(&ip->src_addr, myip, 4);
	memcpy(&ip->dst_addr, dst_addr, 4);
	
	ip->chksum = htons(0);
	sum = checksum(packet, 20);
	ip->chksum = htons(sum);

#ifdef USTACK_DEBUG_IP
	printf("[DEBUG] IP packet out: src %d.%d.%d.%d, dst %d.%d.%d.%d, size %d, sum %04x\n",
		ip->src_addr[0], ip->src_addr[1], ip->src_addr[2], ip->src_addr[3],
		ip->dst_addr[0], ip->dst_addr[1], ip->dst_addr[2], ip->dst_addr[3],
		ntohs(ip->len), ntohs(ip->chksum));
#endif

	val = netif_send(packet, len);

	return val;
}

int32_t ip_in(uint8_t dst_addr[4], uint8_t *packet, uint16_t len)
{
	int32_t val = -1;
	uint16_t chksum;
	struct ip_s *ip = (struct ip_s *)packet;

	if (ip->ver != (IP_VER_IHL >> 8)) {
#ifdef USTACK_DEBUG_ERR
		printf("[ERROR] IP version / options error\n");
#endif
		return -1;							/* IP version / options error (not supported) */
	}
	if (ntohs(ip->flags_off) & (IP_FLAG_MOREFRAG | IP_FRAGOFS_MASK)) {
#ifdef USTACK_DEBUG_ERR
		printf("[ERROR] IP fragmented packets not supported\n");
#endif
		return -1;							/* IP fragmented packets not supported */
	}
	if (ip->ttl == 0) {
#ifdef USTACK_DEBUG_ERR
		printf("[ERROR] IP TTL expired\n");
#endif
		return -1;							/* TP TTL has expired */
	}

	if (!ip_addr_cmp((uint8_t *)&ip->dst_addr, dst_addr)) {
		if (!ip_addr_isbroadcast((uint8_t *)&ip->dst_addr, mynm)) {
			if (!ip_addr_ismulticast((uint8_t *)&ip->dst_addr)) {
#ifdef USTACK_DEBUG_ERR
				printf("[ERROR] IP destination address error\n");
#endif
				return -1;					/* IP destination address error */
			}
		}
	}

	chksum = ntohs(ip->chksum);
	ip->chksum = htons(0);
	if (chksum != checksum(packet, 20)) {
#ifdef USTACK_DEBUG_ERR
		printf("[ERROR] IP checksum error\n");
#endif		
		return -1;							/* IP checksum error */
	}
	ip->chksum = htons(chksum);

#ifdef USTACK_DEBUG_IP
	printf("[DEBUG] IP packet in: src %d.%d.%d.%d, dst %d.%d.%d.%d, size %d, sum %04x\n",
		ip->src_addr[0], ip->src_addr[1], ip->src_addr[2], ip->src_addr[3],
		ip->dst_addr[0], ip->dst_addr[1], ip->dst_addr[2], ip->dst_addr[3],
		ntohs(ip->len), ntohs(ip->chksum));
#endif

	switch (ip->proto) {
	case IP_PROTO_ICMP:
		val = icmp_echo_reply(packet, len);
		break;
	case IP_PROTO_UDP:
		val = udp_in(packet);
		break;
	default:
		if (ip_callback)
			val = ip_callback(packet);
#ifdef USTACK_DEBUG_ERR
		if (val < 0) 
			printf("[ERROR] IP protocol error\n");
#endif
	}

	return val;
}

void ip_set_callback(int32_t (*callback)(uint8_t *packet))
{
	ip_callback = callback;
}
