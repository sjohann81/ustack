/* file:          icmp.c
 * description:   ICMP echo protocol implementation (RFC792)
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdio.h>
#include <string.h>
#include "include/ustack.h"


int32_t icmp_echo_reply(uint8_t *packet, uint16_t len)
{
	uint8_t dst_addr[4];
	uint16_t chksum, val;
	struct ip_icmp_s *icmp = (struct ip_icmp_s *)packet;

	switch(icmp->icmp.type) {
	case ICMP_ECHO :
		icmp->icmp.type = ICMP_ECHO_REPLY;
		chksum = ntohs(icmp->icmp.chksum);
		if (chksum > (0xffff - (ICMP_ECHO << 8))) {
			chksum += (ICMP_ECHO << 8) + 1;
		} else {
			chksum += (ICMP_ECHO << 8);
		}
		icmp->icmp.chksum = htons(chksum);
		memcpy(dst_addr, &icmp->ip.src_addr, 4);

#ifdef USTACK_DEBUG_ICMP
		printf("[DEBUG] ICMP echo reply to %d.%d.%d.%d\n",
		icmp->ip.src_addr[0], icmp->ip.src_addr[1],
		icmp->ip.src_addr[2], icmp->ip.src_addr[3]);
#endif

		val = ip_out(dst_addr, -1, packet, len);

		return val;
	case ICMP_ECHO_REPLY :
	
#ifdef USTACK_DEBUG_ICMP
		printf("[DEBUG] ICMP echo reply from %d.%d.%d.%d\n",
		icmp->ip.src_addr[0], icmp->ip.src_addr[1],
		icmp->ip.src_addr[2], icmp->ip.src_addr[3]);
#endif

		return 0;
	default :						/* ICMP protocol error (other protocol / not implemented) */
#ifdef USTACK_DEBUG_ERR
		printf("[ERROR] ICMP protocol error\n");
#endif
		return -1;
	}
}

int32_t icmp_echo_request(uint8_t dst_addr[4], uint8_t *packet)
{
	uint16_t chksum, val;
	struct ip_icmp_s *icmp = (struct ip_icmp_s *)packet;

	icmp->icmp.type = ICMP_ECHO;
	icmp->icmp.code = 0;
	icmp->icmp.chksum = htons(0);
	icmp->icmp.id = htons(0x9669);
	icmp->icmp.seqno = htons(0x0001);
	memset(&icmp + sizeof(struct ip_icmp_s), 0x55, 16);
	chksum = checksum((uint8_t *)&icmp->icmp, ICMP_HDR_SIZE + 16);
	icmp->icmp.chksum = htons(chksum);

#ifdef USTACK_DEBUG_ICMP
	printf("[DEBUG] ICMP echo request to %d.%d.%d.%d\n",
		dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3]);
#endif

	val = ip_out(dst_addr, IP_PROTO_ICMP, packet, sizeof(struct ip_icmp_s) + 16);
	
	return val;
}
