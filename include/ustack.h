/* file:          ustack.h
 * description:   ustack definitions
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdint.h>

/* general configuration definitions */
#define FRAME_SIZE		1518		/* must be even! between 594 (MTU=576) and 1518 (MTU=1500, max. for Ethernet) */
#define ARP_CACHE_SIZE		16		/* number of entries on the ARP cache. must be powers of two! */
#define IP_CFG_PING		113		/* ping payload magic size for IP configuration */
#define UDP_DEFAULT_PORT	30168		/* UDP default application port */

/* Ethernet link definitions */
#define FRAME_ARP		0x0806		// ARP frame
#define FRAME_IP		0x0800		// IPv4
#define FRAME_IEEE		0x8100 		// IEEE 802.1Q
#define FRAME_IPV6		0x86dd		// IPv6
#define FRAME_TEST		0x8888

struct eth_s {
	uint8_t dst_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
};

/* ARP definitions */
#define HARDW_ETH10		1		// hardware-type 10Mbps Ethernet
#define IP_HLEN_PLEN		0x0604		// MAC = 6 byte long, IP = 4 byte long
#define OP_ARP_REQUEST		1		// operations for ARP-frames
#define OP_ARP_ANSWER		2

struct arp_entry_s {
	uint8_t ip[4];
	uint8_t mac[6];
};

struct arp_s {
	uint16_t hw_type;
	uint16_t proto_type;
	uint16_t hw_len_proto_len;
	uint16_t operation;
	uint8_t sender_ha[6];
	uint8_t sender_pa[4];
	uint8_t target_ha[6];
	uint8_t target_pa[4];
};

/* BOOTP definitions */
#define BOOTP_TRIES		2
#define BOOTP_WAIT		5000
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68
#define BOOTREPLY		2
#define BOOTREQUEST		1
#define BPFLAG_BROADCAST	(1 << 15)
#define HTYPE_ETHERNET		1
#define TAG_END			255
#define TAG_SUBNET_MASK		1
#define TAG_GATEWAY		3

struct bootp_s {
	uint8_t bp_op;			/* packet opcode type */
	uint8_t bp_htype;		/* hardware addr type */
	uint8_t bp_hlen;		/* hardware addr length */
	uint8_t bp_hops;		/* gateway hops */
	uint8_t bp_xid[4];		/* transaction ID */
	uint16_t bp_secs;		/* seconds since boot began */
	uint16_t bp_flags;		/* RFC1532 broadcast, etc. */
	uint8_t bp_ciaddr[4];		/* client IP address */
	uint8_t bp_yiaddr[4];		/* 'your' IP address */
	uint8_t bp_siaddr[4];		/* (next) server IP address */
	uint8_t bp_riaddr[4];		/* relay IP address */
	uint8_t bp_chaddr[16];		/* client hardware address */
	int8_t bp_sname[64];		/* server host name */
	int8_t bp_file[128];		/* boot file name */
	uint8_t bp_vend[64];		/* vendor-specific area */
};

/* IP layer definitions */
#define IP_VER_IHL		0x4500
#define IP_TOS_D		0x0010
#define IP_TOS_T		0x0008
#define IP_TOS_R		0x0004

#define IP_FLAG_DONTFRAG	0x4000
#define IP_FLAG_MOREFRAG	0x2000
#define IP_FRAGOFS_MASK		0x1fff

#define IP_DEFAULT_TTL		64

#define IP_PROTO_ICMP		1
#define IP_PROTO_IGMP		2
#define IP_PROTO_TCP		6
#define IP_PROTO_IGRP		9
#define IP_PROTO_UDP		17
#define IP_PROTO_GRE		47
#define IP_PROTO_ESP		50
#define IP_PROTO_AH		51
#define IP_PROTO_SKIP		57
#define IP_PROTO_EIGRP		88
#define IP_PROTO_OSPF		89
#define IP_PROTO_L2TP		115
#define IP_PROTO_SCTF		132

struct ip_s {
	uint8_t ver;			/* version, header length */
	uint8_t tos;			/* type of service */
	int16_t len;			/* total length */
	uint16_t id;			/* identification */
	int16_t flags_off;		/* flags and fragment offset fields */
	uint8_t ttl;			/* time to live */
	uint8_t proto;			/* protocol */
	uint16_t chksum;		/* checksum */
	uint8_t src_addr[4];		/* source address */
	uint8_t dst_addr[4];		/* destination address */	
};

/* ICMP definitions */
#define ICMP_ECHO_REPLY		0
#define ICMP_ECHO		8
#define ICMP_HDR_SIZE		8

struct icmp_s {
	uint8_t type;
	uint8_t code;
	uint16_t chksum;
	uint16_t id;
	uint16_t seqno;
};

/* UDP definitions */
struct udp_s {
	uint16_t src_port;		/* source port */
	uint16_t dst_port;		/* destination port */
	uint16_t len;			/* total length */
	uint16_t chksum;		/* checksum */
};

/* standard ports */
#define PORT_ECHO		7
#define PORT_DISCARD		9
#define PORT_DAYTIME		13
#define PORT_QOTD		17
#define PORT_CHARGEN		19


/* composition of headers */
struct frame_netif_s {
	struct eth_s ethernet;
	union {
		struct ip_s ip;
		struct arp_s arp;
	} payload;
};

struct eth_arp_s {
	struct eth_s ethernet;
	struct arp_s arp;
};

struct eth_ip_udp_bootp_s {
	struct eth_s ethernet;
	struct ip_s ip;
	struct udp_s udp;
	struct bootp_s bootp;
};

struct eth_ip_s {
	struct eth_s ethernet;
	struct ip_s ip;
};

struct ip_icmp_s {
	struct ip_s ip;
	struct icmp_s icmp;
};

struct ip_udp_s {
	struct ip_s ip;
	struct udp_s udp;
};

extern struct arp_entry_s arp_cache[ARP_CACHE_SIZE];
extern uint8_t eth_frame[FRAME_SIZE];
extern uint8_t mymac[6];
extern uint8_t myip[4];
extern uint8_t mynm[4];
extern uint8_t mygw[4];

/* utility macros and functions */
#ifndef USTACK_BIG_ENDIAN

#define htons(n) (((((uint16_t)(n) & 0xFF)) << 8) | (((uint16_t)(n) & 0xFF00) >> 8))
#define ntohs(n) (((((uint16_t)(n) & 0xFF)) << 8) | (((uint16_t)(n) & 0xFF00) >> 8))

#define htonl(n) (((((uint32_t)(n) & 0xFF)) << 24) | \
		((((uint32_t)(n) & 0xFF00)) << 8) | \
		((((uint32_t)(n) & 0xFF0000)) >> 8) | \
		((((uint32_t)(n) & 0xFF000000)) >> 24))

#define ntohl(n) (((((uint32_t)(n) & 0xFF)) << 24) | \
		((((uint32_t)(n) & 0xFF00)) << 8) | \
		((((uint32_t)(n) & 0xFF0000)) >> 8) | \
		((((uint32_t)(n) & 0xFF000000)) >> 24))

#else

#define htons(n) (n)
#define ntohs(n) (n)
#define htonl(n) (n)
#define ntohl(n) (n)

#endif

uint16_t checksum(uint8_t *buf, int32_t len);
int32_t is_broadcast_mac(uint8_t *frame);
int32_t is_any_mac(uint8_t *frame);
int32_t is_local_mac(uint8_t *frame);
int32_t ip_addr_maskcmp(uint8_t addr1[4], uint8_t addr2[4], uint8_t mask[4]);
int32_t ip_addr_cmp(uint8_t addr1[4], uint8_t addr2[4]);
int32_t ip_addr_isany(uint8_t addr[4]);
int32_t ip_addr_isbroadcast(uint8_t addr[4], uint8_t mask[4]);
int32_t ip_addr_ismulticast(uint8_t addr[4]);
int32_t hexdump(uint8_t *buf, uint32_t size);

/* layer 1 */
extern void tun_init();
extern void tun_deinit();

extern void en_ll_output(uint8_t *frame, uint16_t size);
extern int32_t en_ll_input(uint8_t *frame);

/* layer 2 */
uint16_t netif_send(uint8_t *packet, uint16_t len);
uint16_t netif_recv(uint8_t *packet);

int32_t arp_reply(uint8_t *frame);
int32_t arp_request(uint8_t *ip, uint8_t *frame);
int32_t arp_update(uint8_t *ip, uint8_t *mac);
int32_t arp_check(uint8_t *ip, uint8_t *mac);

void bootp_make_request(uint8_t *frame);
uint16_t bootp_handle_reply(uint8_t *frame);
int32_t bootp_boot(uint8_t *packet);

/* layer 3 */
int32_t ip_out(uint8_t dst_addr[4], int16_t proto, uint8_t *packet, uint16_t len);
int32_t ip_in(uint8_t dst_addr[4], uint8_t *packet, uint16_t len);
void ip_set_callback(int32_t (*callback)(uint8_t *packet));

int32_t icmp_echo_reply(uint8_t *packet, uint16_t len);
int32_t icmp_echo_request(uint8_t dst_addr[4], uint8_t *packet);

/* layer 4 */
int32_t udp_out(uint8_t dst_addr[4], uint16_t src_port, uint16_t dst_port, uint8_t *packet, uint16_t len);
int32_t udp_in(uint8_t *packet);
void udp_set_callback(int32_t (*callback)(uint8_t *packet));
