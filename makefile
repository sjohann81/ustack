# compiler flags:
#
# USTACK_BIG_ENDIAN		configures ustack to work on big endian machines
# USTACK_TAP_ADDR		ustack TUN/TAP interface address
# USTACK_TAP_ROUTE		ustack TUN/TAP interface default route
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

DFLAGS = 	-DUSTACK_TAP_ADDR=\"172.31.69.1/24\" \
		-DUSTACK_TAP_ROUTE=\"172.31.69.0/24\" \
		-DUSTACK_DEBUG_BOOTP \
		-DUSTACK_DEBUG_ARP \
		-DUSTACK_DEBUG_IP \
		-DUSTACK_DEBUG_ICMP \
		-DUSTACK_DEBUG_UDP \
		-DUSTACK_DEBUG_ERR
		
all:
	gcc $(DFLAGS) -O2 -c utils.c -o utils.o
	gcc $(DFLAGS) -O2 -c tuntap_if.c -o tuntap_if.o
	gcc $(DFLAGS) -O2 -c eth_netif.c -o ethnet_if.o
	gcc $(DFLAGS) -O2 -c bootp.c -o bootp.o
	gcc $(DFLAGS) -O2 -c arp.c -o arp.o
	gcc $(DFLAGS) -O2 -c ip.c -o ip.o
	gcc $(DFLAGS) -O2 -c icmp.c -o icmp.o
	gcc $(DFLAGS) -O2 -c udp.c -o udp.o
	gcc $(DFLAGS) -O2 -c main.c -o main.o

	gcc -O2 utils.o tuntap_if.o ethnet_if.o bootp.o arp.o ip.o icmp.o udp.o main.o -o ustack

dump:
	tcpdump -l -n -S -XX -s 0 -vv -i sl0
	
forwarding:
	echo 1 > /proc/sys/net/ipv4/ip_forward

clean:
	rm -rf *~ *o ustack
