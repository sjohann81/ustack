/* file:          bootp.c
 * description:   BOOTP protocol implementation (RFC951)
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

/* sudo apt-get install inetutils-inetd
 *
 * edit /etc/inetd.conf e remove the comment on the line:
 * bootps  dgram  udp  wait  root  /usr/sbin/bootpd  bootpd
 *
 * edit /etc/bootptab and set individual entries:
 * client:ip=192.168.1.90:sm=255.255.255.0:gw=192.168.1.1:ha=0123456789AB:
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/ustack.h"

static uint8_t mincookie[] = {99, 130, 83, 99, 255};
static uint8_t replycookie[] = {0x63, 0x82, 0x53, 0x63};
static uint8_t temp_xid[4];


void bootp_make_request(uint8_t *frame)
{
	int32_t i;
	uint8_t c;
	struct eth_ip_udp_bootp_s *bootp = (struct eth_ip_udp_bootp_s *)frame;

	memset(&bootp->ip, 0, sizeof(struct ip_s));
	memset(&bootp->bootp, 0, sizeof(struct bootp_s));

	bootp->bootp.bp_op = BOOTREQUEST;
	bootp->bootp.bp_htype = HTYPE_ETHERNET;
	bootp->bootp.bp_hlen = 6;
	for (i = 0; i < 4; i++) {
		c = rand() & 0xff;
		bootp->bootp.bp_xid[i] = c;
		temp_xid[i] = c;
	}

	bootp->bootp.bp_flags |= htons(BPFLAG_BROADCAST);      /* request broadcast reply */
	memcpy(bootp->bootp.bp_chaddr, mymac, 6);

	memcpy(bootp->bootp.bp_vend, mincookie, sizeof(mincookie));
}

uint16_t bootp_handle_reply(uint8_t *frame)
{
	int32_t i;
	struct eth_ip_udp_bootp_s *bootp = (struct eth_ip_udp_bootp_s *)frame;

	if (bootp->bootp.bp_op != BOOTREPLY)
		return 0;

	if (bootp->bootp.bp_htype != HTYPE_ETHERNET)
		return 0;

	for (i = 0; i < 4; i++){
		if (bootp->bootp.bp_xid[i] != temp_xid[i])
			return 0;

		if (bootp->bootp.bp_vend[i] != replycookie[i])
			return 0;
	}

	/* extract ip address, netmask and gateway */
	memcpy(&myip[0], bootp->bootp.bp_yiaddr, 4);

	uint8_t *ptr = bootp->bootp.bp_vend + 4;
	while (*ptr != TAG_END) {
		switch (*ptr){
		case TAG_SUBNET_MASK:
			memcpy(&mynm[0], &ptr[2], 4);
			break;
		case TAG_GATEWAY:
			memcpy(&mygw[0], &ptr[2], 4);
			break;
		}

		ptr = ptr + ptr[1] + 2;
	}

	return 1;
}

int32_t bootp_boot(uint8_t *packet)
{
	uint8_t addr[4] = {255, 255, 255, 255};
	uint8_t *frame;
	struct eth_ip_udp_bootp_s *bootp;
	int32_t tries = 0, ll_len;
	uint16_t type, dest_port;

	frame = packet - sizeof(struct eth_s);
	bootp = (struct eth_ip_udp_bootp_s *)frame;

	while (1) {
#ifdef USTACK_DEBUG_BOOTP
		printf("[DEBUG]: bootp_boot(), requesting interface configuration\n");
#endif
		bootp_make_request(frame);
		udp_out(addr, IPPORT_BOOTPC, IPPORT_BOOTPS, packet, sizeof(struct udp_s) + sizeof(struct bootp_s));
		sleep(1);
		
		ll_len = en_ll_input(frame);

		if (ll_len > 0){
			if (is_broadcast_mac(frame) || is_any_mac(frame)) {
				type = ntohs(bootp->ethernet.type);
				if (type == FRAME_IP) {
					dest_port = ntohs(bootp->udp.dst_port);
					if (bootp->ip.proto == IP_PROTO_UDP && dest_port == IPPORT_BOOTPC) {
						if (bootp_handle_reply(frame)) {
#ifdef USTACK_DEBUG_BOOTP
							printf("[DEBUG]: bootp_boot(), interface configured by BOOTP\n");
							printf("[DEBUG]: IP: %d.%d.%d.%d, NM: %d.%d.%d.%d, GW: %d.%d.%d.%d\n",
								myip[0], myip[1], myip[2], myip[3],
								mynm[0], mynm[1], mynm[2], mynm[3],
								mygw[0], mygw[1], mygw[2], mygw[3]
							);
#endif
							return 1;
						}
					}
				}
			}
		}

		if (tries++ == BOOTP_TRIES) break;
	}

	return 0;
}
