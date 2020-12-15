/* file:          eth_netif.c
 * description:   ethernet link layer abstractions
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdio.h>
#include <string.h>
#include "include/ustack.h"


/*
 * network interface send() / receive()
 *
 * -abstract the link layer
 * -handle interface specific protocols
 * -handle broadcast frames
 * -handle ARP protocol
 */
uint16_t netif_send(uint8_t *packet, uint16_t len)
{
	uint8_t mac[6], ip[4];
	int32_t l, arp_r = 0;
	uint8_t *frame = (uint8_t *)(packet - sizeof(struct eth_s));
	struct frame_netif_s *frame_s = (struct frame_netif_s *)(packet - sizeof(struct eth_s));

	memcpy(ip, &frame_s->payload.ip.dst_addr, 4);

	if (ip_addr_isany(ip) || ip_addr_isbroadcast(ip, mynm)) {
		memset(mac, 0xff, 6);
		arp_r = 1;
	} else {
		if (ip_addr_ismulticast(ip)){
		    /* Hash IP multicast address to MAC address. */
		    mac[0] = 0x01; mac[1] = 0x0; mac[2] = 0x5e;
		    mac[3] = ip[1] & 0x7f; mac[4] = ip[2]; mac[5] = ip[3];
		    arp_r = 1;
		} else {
			if ((myip[0] | myip[1] | myip[2] | myip[3]) == 0) {
				arp_r = 1;
			} else {
				if (!ip_addr_maskcmp(ip, myip, mynm))
					memcpy(ip, mygw, 4);

				arp_r = arp_check(ip, mac);
			}
		}
	}

	if (arp_r) {
		memcpy(&frame_s->ethernet.src_addr, mymac, 6);
		memcpy(&frame_s->ethernet.dst_addr, mac, 6);
		frame_s->ethernet.type = htons(FRAME_IP);

#ifdef USTACK_DEBUG_ETH
		printf("[DEBUG] ETH frame out: dst %02x:%02x:%02x:%02x:%02x:%02x src %02x:%02x:%02x:%02x:%02x:%02x type %04x\n",
			frame_s->ethernet.dst_addr[0], frame_s->ethernet.dst_addr[1], frame_s->ethernet.dst_addr[2],
			frame_s->ethernet.dst_addr[3], frame_s->ethernet.dst_addr[4], frame_s->ethernet.dst_addr[5],
			frame_s->ethernet.src_addr[0], frame_s->ethernet.src_addr[1], frame_s->ethernet.src_addr[2], 
			frame_s->ethernet.src_addr[3], frame_s->ethernet.src_addr[4], frame_s->ethernet.src_addr[5],
			ntohs(frame_s->ethernet.type));
#endif

		en_ll_output(frame, len + sizeof(struct eth_s));

		return len + sizeof(struct eth_s);
	} else {
		l = arp_request(ip, frame);

#ifdef USTACK_DEBUG_ETH
		printf("[DEBUG] ETH frame out: dst %02x:%02x:%02x:%02x:%02x:%02x src %02x:%02x:%02x:%02x:%02x:%02x type %04x\n",
			frame_s->ethernet.dst_addr[0], frame_s->ethernet.dst_addr[1], frame_s->ethernet.dst_addr[2],
			frame_s->ethernet.dst_addr[3], frame_s->ethernet.dst_addr[4], frame_s->ethernet.dst_addr[5],
			frame_s->ethernet.src_addr[0], frame_s->ethernet.src_addr[1], frame_s->ethernet.src_addr[2], 
			frame_s->ethernet.src_addr[3], frame_s->ethernet.src_addr[4], frame_s->ethernet.src_addr[5],
			ntohs(frame_s->ethernet.type));
#endif

		en_ll_output(frame, l + sizeof(struct eth_s));

		return 0;
	}
}

uint16_t netif_recv(uint8_t *packet)
{
	int32_t len = 0, ll_len;
	uint16_t type;
	uint8_t *frame = (uint8_t *)(packet - sizeof(struct eth_s));
	struct frame_netif_s *frame_s = (struct frame_netif_s *)(packet - sizeof(struct eth_s));

	ll_len = en_ll_input(frame);

	if (ll_len > 0){
#ifdef USTACK_DEBUG_ETH
		printf("[DEBUG] ETH frame in: dst %02x:%02x:%02x:%02x:%02x:%02x src %02x:%02x:%02x:%02x:%02x:%02x type %04x\n",
			frame_s->ethernet.dst_addr[0], frame_s->ethernet.dst_addr[1], frame_s->ethernet.dst_addr[2],
			frame_s->ethernet.dst_addr[3], frame_s->ethernet.dst_addr[4], frame_s->ethernet.dst_addr[5],
			frame_s->ethernet.src_addr[0], frame_s->ethernet.src_addr[1], frame_s->ethernet.src_addr[2], 
			frame_s->ethernet.src_addr[3], frame_s->ethernet.src_addr[4], frame_s->ethernet.src_addr[5],
			ntohs(frame_s->ethernet.type));
#endif
		if (is_local_mac(frame) || is_broadcast_mac(frame) || is_any_mac(frame)) {
			type = ntohs(frame_s->ethernet.type);

			switch(type){
			case FRAME_ARP:
				if (ntohs(frame_s->payload.arp.hw_type) == HARDW_ETH10){
					if (ntohs(frame_s->payload.arp.proto_type) == FRAME_IP){
						if (ntohs(frame_s->payload.arp.hw_len_proto_len) == IP_HLEN_PLEN){
							if (ntohs(frame_s->payload.arp.operation) == OP_ARP_REQUEST) {
								if (!memcmp(&frame_s->payload.arp.target_pa, myip, 4)){
									len = arp_reply(frame);
#ifdef USTACK_DEBUG_ETH
									printf("[DEBUG] ETH frame out: dst %02x:%02x:%02x:%02x:%02x:%02x src %02x:%02x:%02x:%02x:%02x:%02x type %04x\n",
										frame_s->ethernet.dst_addr[0], frame_s->ethernet.dst_addr[1], frame_s->ethernet.dst_addr[2],
										frame_s->ethernet.dst_addr[3], frame_s->ethernet.dst_addr[4], frame_s->ethernet.dst_addr[5],
										frame_s->ethernet.src_addr[0], frame_s->ethernet.src_addr[1], frame_s->ethernet.src_addr[2], 
										frame_s->ethernet.src_addr[3], frame_s->ethernet.src_addr[4], frame_s->ethernet.src_addr[5],
										ntohs(frame_s->ethernet.type));
#endif
									en_ll_output(frame, len);

									return 0;
								}
							}

							if (ntohs(frame_s->payload.arp.operation) == OP_ARP_ANSWER)
								arp_update((uint8_t *)&frame_s->payload.arp.sender_pa, (uint8_t *)&frame_s->payload.arp.sender_ha);
						}
					}
				}
				break;
			case FRAME_IP:
				len = ntohs(frame_s->payload.ip.len);

				if (ip_addr_isany(myip)){
					if (frame_s->payload.ip.proto == IP_PROTO_ICMP && len == IP_CFG_PING + sizeof(struct ip_icmp_s)){		/* configure the IP address */
						memcpy(myip, frame_s->payload.ip.dst_addr, 4);
						memcpy(mygw, frame_s->payload.ip.src_addr, 4);

						switch (myip[0]) {
						case 1 ... 127:
							mynm[0] = 255; mynm[1] = 0; mynm[2] = 0; mynm[3] = 0;
							break;
						case 128 ... 191:
							mynm[0] = 255; mynm[1] = 255; mynm[2] = 0; mynm[3] = 0;
							break;
						case 192 ... 223:
							mynm[0] = 255; mynm[1] = 255; mynm[2] = 255; mynm[3] = 0;
							break;
						default:
							mynm[0] = 0; mynm[1] = 0; mynm[2] = 0; mynm[3] = 0;
						}
#ifdef USTACK_DEBUG_IP
						printf("[DEBUG] interface configured by ping\n");
						printf("[DEBUG] ip %d.%d.%d.%d, netmask %d.%d.%d.%d, gateway %d.%d.%d.%d\n",
							myip[0], myip[1], myip[2], myip[3],
							mynm[0], mynm[1], mynm[2], mynm[3],
							mygw[0], mygw[1], mygw[2], mygw[3]);
#endif
					}

					return 0;
				}
				break;
			case FRAME_IEEE:
				break;
			case FRAME_IPV6:
				break;
			case FRAME_TEST:
				break;
			default:
				break;
			}
		}
	}

	if (len < 0)
		return 0;
	else
		return len;
}
