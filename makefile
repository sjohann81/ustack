# application flags:
# USTACK_IP_ADDR		static configuration of IP address
# USTACK_NETMASK		static configuration of network mask
# USTACK_GW_ADDR		static configuration of gateway address
# USTACK_BIG_ENDIAN		configures ustack to work on big endian machines
# USTACK_TAP_ADDR		TUN/TAP interface address
# USTACK_TAP_ROUTE		TUN/TAP interface default route
#
# debug compiler flags:
# USTACK_DEBUG_FRAMES		(layer-1) low level frame level debug (hexdump)
# USTACK_DEBUG_ETH		(layer-2) Ethernet level debug (informative)
# USTACK_DEBUG_BOOTP		(layer-2) BOOTP level debug (informative)
# USTACK_DEBUG_ARP		(layer-2) ARP level debug (informative)
# USTACK_DEBUG_IP		(layer-3) IP level debug (informative)
# USTACK_DEBUG_ICMP		(layer-3) ICMP level debug (informative)
# USTACK_DEBUG_UDP		(layer-4) UDP level debug (informative)
# USTACK_DEBUG_ERR		(all layers) error debug messages

AFLAGS = 	-DUSTACK_IP_ADDR=\"172.31.69.20\" \
		-DUSTACK_NETMASK=\"255.255.255.0\" \
		-DUSTACK_GW_ADDR=\"172.31.69.1\" \
		-DUSTACK_TAP_ADDR=\"172.31.69.1/24\" \
		-DUSTACK_TAP_ROUTE=\"172.31.69.0/24\" 
		
DFLAGS =	-DUSTACK_DEBUG_BOOTP \
		-DUSTACK_DEBUG_ARP \
		-DUSTACK_DEBUG_IP \
		-DUSTACK_DEBUG_ICMP \
		-DUSTACK_DEBUG_UDP \
		-DUSTACK_DEBUG_ERR

CFLAGS =	-O2 -Wall -c

FLAGS =		$(AFLAGS) $(DFLAGS) $(CFLAGS)

all:
	gcc $(FLAGS) utils.c -o utils.o
	gcc $(FLAGS) tuntap_if.c -o tuntap_if.o
	gcc $(FLAGS) eth_netif.c -o ethnet_if.o
	gcc $(FLAGS) bootp.c -o bootp.o
	gcc $(FLAGS) arp.c -o arp.o
	gcc $(FLAGS) ip.c -o ip.o
	gcc $(FLAGS) icmp.c -o icmp.o
	gcc $(FLAGS) udp.c -o udp.o
	gcc $(FLAGS) main.c -o main.o

	gcc -O2 utils.o tuntap_if.o ethnet_if.o bootp.o arp.o ip.o icmp.o udp.o main.o -o ustack

dump:
	tcpdump -l -n -S -XX -s 0 -vv -i sl0
	
forwarding:
	echo 1 > /proc/sys/net/ipv4/ip_forward

clean:
	rm -rf *~ *o ustack
