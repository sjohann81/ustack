#include <stdio.h>
#include <string.h>
#include "include/ustack.h"


uint8_t eth_frame[FRAME_SIZE];
uint8_t mymac[6];
uint8_t myip[4];
uint8_t mynm[4];
uint8_t mygw[4];


int32_t app_udp_handler(uint8_t *packet)
{
	uint8_t dst_addr[4];
	uint16_t src_port, dst_port;
	struct ip_udp_s *udp = (struct ip_udp_s *)packet;
	uint8_t msg[] = "Hello world!";

	src_port = ntohs(udp->udp.src_port);
	dst_port = ntohs(udp->udp.dst_port);

	if (dst_port == UDP_DEFAULT_PORT) {
		printf("%s\n", msg);
		
		memcpy(dst_addr, udp->ip.src_addr, 4);
		memcpy(packet + sizeof(struct ip_udp_s), msg, sizeof(msg));
		udp_out(dst_addr, dst_port, src_port, packet, sizeof(struct udp_s) + sizeof(msg));
	}
	
	return 0;
}

void ethernet_exec(uint8_t *packet)
{
	uint16_t len;

	len = netif_recv(packet);

	if (len > 0){
		ip_in(myip, packet, len);
	}
}

int main(void)
{
	uint8_t *packet = eth_frame + sizeof(struct eth_s);
	
	if_setup();
	config(myip, USTACK_IP_ADDR);
	config(mynm, USTACK_NETMASK);
	config(mygw, USTACK_GW_ADDR);
	udp_set_callback(app_udp_handler);

	printf("[DEBUG] ip %d.%d.%d.%d, netmask %d.%d.%d.%d, gateway %d.%d.%d.%d\n",
		myip[0], myip[1], myip[2], myip[3],
		mynm[0], mynm[1], mynm[2], mynm[3],
		mygw[0], mygw[1], mygw[2], mygw[3]);
		
	while (1) {
		ethernet_exec(packet);
	}

	if_deinit();
	
	return 0;
}

