/* file:          arp.c
 * description:   ARP protocol implementation (RFC826 and RFC5227)
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdio.h>
#include <string.h>
#include "include/ustack.h"

struct arp_entry_s arp_cache[ARP_CACHE_SIZE];
static uint32_t arp_index = 0;

int32_t arp_reply(uint8_t *frame)
{
	uint8_t mac[6], ip[4];
	struct eth_arp_s *arp = (struct eth_arp_s *)frame;
	
	/* ethernet */
	memcpy(mac, &arp->ethernet.src_addr, 6);
	memcpy(&arp->ethernet.src_addr, mymac, 6);
	memcpy(&arp->ethernet.dst_addr, mac, 6);
	arp->ethernet.type = htons(FRAME_ARP);
	
	/* ARP */
	arp->arp.hw_type = htons(HARDW_ETH10);
	arp->arp.proto_type = htons(FRAME_IP);
	arp->arp.hw_len_proto_len = htons(IP_HLEN_PLEN);
	arp->arp.operation = htons(OP_ARP_ANSWER);
	memcpy(ip, &arp->arp.sender_pa, 4);
	memcpy(&arp->arp.sender_ha, mymac, 6);
	memcpy(&arp->arp.sender_pa, myip, 4);
	memcpy(&arp->arp.target_ha, mac, 6);
	memcpy(&arp->arp.target_pa, ip, 4);

#ifdef USTACK_DEBUG_ARP
	printf("[DEBUG] ARP reply: %d.%d.%d.%d is at %02x:%02x:%02x:%02x:%02x:%02x, to %d.%d.%d.%d\n",
		arp->arp.sender_pa[0], arp->arp.sender_pa[1], arp->arp.sender_pa[2], arp->arp.sender_pa[3],
		arp->arp.sender_ha[0], arp->arp.sender_ha[1], arp->arp.sender_ha[2],
		arp->arp.sender_ha[3], arp->arp.sender_ha[4], arp->arp.sender_ha[5],
		arp->arp.target_pa[0], arp->arp.target_pa[1], arp->arp.target_pa[2], arp->arp.target_pa[3]);
#endif

	return sizeof(struct eth_arp_s);
}

int32_t arp_request(uint8_t *ip, uint8_t *frame)
{
	struct eth_arp_s *arp = (struct eth_arp_s *)frame;
	
	/* ethernet */
	memcpy(&arp->ethernet.src_addr, mymac, 6);
	memset(&arp->ethernet.dst_addr, 0xff, 6);
	arp->ethernet.type = htons(FRAME_ARP);
	
	/* ARP */
	arp->arp.hw_type = htons(HARDW_ETH10);
	arp->arp.proto_type = htons(FRAME_IP);
	arp->arp.hw_len_proto_len = htons(IP_HLEN_PLEN);
	arp->arp.operation = htons(OP_ARP_REQUEST);
	memcpy(&arp->arp.sender_ha, mymac, 6);
	memcpy(&arp->arp.sender_pa, myip, 4);
	memset(&arp->arp.target_ha, 0x00, 6);
	memcpy(&arp->arp.target_pa, ip, 4);

#ifdef USTACK_DEBUG_ARP
	printf("[DEBUG] ARP request: where is %d.%d.%d.%d\n",
		arp->arp.target_pa[0], arp->arp.target_pa[1], arp->arp.target_pa[2], arp->arp.target_pa[3]);
#endif

	return sizeof(struct eth_arp_s);
}

int32_t arp_update(uint8_t *ip, uint8_t *mac)
{
	int32_t i;
	
	for (i = 0; i < ARP_CACHE_SIZE; i++)
		if (!memcmp(ip, arp_cache[i].ip, 4)) break;

#ifdef USTACK_DEBUG_ARP
	printf("[DEBUG] ARP update: %d.%d.%d.%d is at %02x:%02x:%02x:%02x:%02x:%02x\n",
		ip[0], ip[1], ip[2], ip[3], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

	if (i < ARP_CACHE_SIZE){
		memcpy(arp_cache[i].mac, mac, 6);
		
		return 1;
	} else {
		memcpy(arp_cache[arp_index].ip, ip, 4);
		memcpy(arp_cache[arp_index].mac, mac, 6);
		i = arp_index;
		arp_index = (arp_index + 1) & (ARP_CACHE_SIZE - 1);

		return 0;
	}
}

int32_t arp_check(uint8_t *ip, uint8_t *mac)
{
	int32_t i;
	
	for (i = 0; i < ARP_CACHE_SIZE; i++)
		if (!memcmp(ip, arp_cache[i].ip, 4)) break;
	
	if (i < ARP_CACHE_SIZE) {
		memcpy(mac, arp_cache[i].mac, 6);
		
		return 1;
	} else {
		return 0;
	}
}
