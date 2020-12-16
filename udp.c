/* file:          udp.c
 * description:   UDP protocol implementation (RFC768) and basic services
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdio.h>
#include <string.h>
#include "include/ustack.h"


static int32_t (*udp_callback)(uint8_t *packet);

static uint16_t udpchksum(uint8_t *packet, uint16_t len)
{
	uint32_t sum = 0;
	uint16_t i, val, *ptr = (uint16_t *)(packet + sizeof(struct ip_s) - 8);

	sum += IP_PROTO_UDP;
	sum += len;

	for (i = 0; i < len + 8 - 1; i += 2) {
		val = *ptr++;
		sum += ntohs(val);
	}

	if (len & 1) {
		val = *ptr;
		sum += ntohs(val) & 0xff00;
	}
	
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return ~sum;
}

int32_t udp_out(uint8_t dst_addr[4], uint16_t src_port, uint16_t dst_port, uint8_t *packet, uint16_t len)
{
	int32_t val;
	uint16_t chksum;
	struct ip_udp_s *udp = (struct ip_udp_s *)packet;
	
	udp->udp.src_port = htons(src_port);
	udp->udp.dst_port = htons(dst_port);
	udp->udp.len = htons(len);
	udp->udp.chksum = htons(0);

	chksum = udpchksum(packet, len);
	udp->udp.chksum = htons(chksum);
	
#ifdef USTACK_DEBUG_UDP
	printf("[DEBUG] UDP packet out: src %d.%d.%d.%d:%d, dst %d.%d.%d.%d:%d, size %d, sum %04x\n",
		udp->ip.src_addr[0], udp->ip.src_addr[1], udp->ip.src_addr[2], udp->ip.src_addr[3], ntohs(udp->udp.src_port),
		dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], ntohs(udp->udp.dst_port),
		ntohs(udp->udp.len), ntohs(udp->udp.chksum));
#endif

	val = ip_out(dst_addr, IP_PROTO_UDP, packet, len + sizeof(struct ip_s));

	return val;
}

int32_t udp_in(uint8_t *packet)
{
	int32_t val = -1;
	uint8_t dst_addr[4];
	uint16_t src_port, dst_port, datalen = 0, chksum;
	struct ip_udp_s *udp = (struct ip_udp_s *)packet;

	chksum = ntohs(udp->udp.chksum);
	datalen = ntohs(udp->udp.len);

	if (chksum) {
		udp->udp.chksum = htons(0);
		if (chksum != udpchksum(packet, datalen)) {
#ifdef USTACK_DEBUG_ERR
			printf("[ERROR] UDP checksum error\n");
#endif	
			return -1;
		}
	}

	src_port = ntohs(udp->udp.src_port);
	dst_port = ntohs(udp->udp.dst_port);
	
#ifdef USTACK_DEBUG_UDP
	printf("[DEBUG] UDP packet in: src %d.%d.%d.%d:%d, dst %d.%d.%d.%d:%d, size %d, sum %04x\n",
		udp->ip.src_addr[0], udp->ip.src_addr[1], udp->ip.src_addr[2], udp->ip.src_addr[3], ntohs(udp->udp.src_port),
		udp->ip.dst_addr[0], udp->ip.dst_addr[1], udp->ip.dst_addr[2], udp->ip.dst_addr[3], ntohs(udp->udp.dst_port),
		ntohs(udp->udp.len), chksum);
#endif

	switch (dst_port) {
	case PORT_ECHO:							/* Echo protocol, RFC862 */
		memcpy(dst_addr, &udp->ip.src_addr, 4);
		udp_out(dst_addr, dst_port, src_port, packet, datalen);
		break;
	case PORT_DISCARD:						/* Discard protocol, RFC863 */
		break;
	default:
		if (udp_callback)
			val = udp_callback(packet);
#ifdef USTACK_DEBUG_ERR
		if (val < 0) 
			printf("[ERROR] UDP destination port error\n");
#endif
		return val;
	}

	return datalen;
}

void udp_set_callback(int32_t (*callback)(uint8_t *packet))
{
	udp_callback = callback;
}
