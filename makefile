# application flags:
# USTACK_BIG_ENDIAN		configures ustack to work on big endian machines
# USTACK_IP_ADDR		static configuration of IP address
# USTACK_NETMASK		static configuration of network mask
# USTACK_GW_ADDR		static configuration of gateway address
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

# configurable section
USTACK_IP_ADDR =	172.31.69.20
USTACK_NETMASK =	255.255.255.0
USTACK_GW_ADDR =	172.31.69.1
USTACK_TAP_ADDR =	172.31.69.1/24
USTACK_TAP_ROUTE =	172.31.69.0/24

BAUD =			57600
SERIAL_DEV0 =		/dev/ttyUSB0

# compiler flags section
AFLAGS = 	-DUSTACK_IP_ADDR=\"$(USTACK_IP_ADDR)\" \
		-DUSTACK_NETMASK=\"$(USTACK_NETMASK)\" \
		-DUSTACK_GW_ADDR=\"$(USTACK_GW_ADDR)\" \
		-DUSTACK_TAP_ADDR=\"$(USTACK_TAP_ADDR)\" \
		-DUSTACK_TAP_ROUTE=\"$(USTACK_TAP_ROUTE)\" 
		
DFLAGS =	-DUSTACK_DEBUG_BOOTP \
		-DUSTACK_DEBUG_ARP \
		-DUSTACK_DEBUG_IP \
		-DUSTACK_DEBUG_ICMP \
		-DUSTACK_DEBUG_UDP \
		-DUSTACK_DEBUG_ERR

CFLAGS =	-O2 -Wall -c

FLAGS =		$(AFLAGS) $(DFLAGS) $(CFLAGS)

SFLAGS =	$(FLAGS) -DSERIAL_DEV0=\"$(SERIAL_DEV0)\" -DBAUD=\"$(BAUD)\"

# rules
ethernet:
	gcc $(FLAGS) utils.c -o utils.o
	gcc $(FLAGS) tuntap_if.c -o tuntap_if.o
	gcc $(FLAGS) eth_netif.c -o eth_netif.o
	gcc $(FLAGS) bootp.c -o bootp.o
	gcc $(FLAGS) arp.c -o arp.o
	gcc $(FLAGS) ip.c -o ip.o
	gcc $(FLAGS) icmp.c -o icmp.o
	gcc $(FLAGS) udp.c -o udp.o
	gcc $(FLAGS) main.c -o main.o

	gcc -O2 utils.o tuntap_if.o eth_netif.o bootp.o arp.o ip.o icmp.o udp.o main.o -o ustack

slip:
	gcc $(SFLAGS) utils.c -o utils.o
	gcc $(SFLAGS) slip_netif.c -o slip_netif.o
	gcc $(SFLAGS) ip.c -o ip.o
	gcc $(SFLAGS) icmp.c -o icmp.o
	gcc $(SFLAGS) udp.c -o udp.o
	gcc $(SFLAGS) main.c -o main.o

	gcc -O2 utils.o slip_netif.o ip.o icmp.o udp.o main.o -o ustack

serial:
	stty ${BAUD} raw cs8 -parenb -crtscts clocal cread ignpar ignbrk -ixon -ixoff -ixany -brkint \
	-icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke -F ${SERIAL_DEV0}

slip_if: serial
	slattach -L -d -p slip -s ${BAUD} ${SERIAL_DEV0} &
	sleep 2
	ifconfig sl0 $(USTACK_GW_ADDR)
	ifconfig sl0 dstaddr $(USTACK_IP_ADDR)
	ifconfig sl0 mtu 576

dump:
	tcpdump -l -n -S -XX -s 0 -vv -i sl0
	
forwarding:
	echo 1 > /proc/sys/net/ipv4/ip_forward

clean:
	rm -rf *~ *o ustack
