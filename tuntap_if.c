/* file:          tuntap_if.c
 * description:   tun/tap interface access / low level
 *                ethernet driver abstraction
 * date:          12/2020
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "include/ustack.h"

static int tun_fd;
static char* dev;

char *tapaddr = USTACK_TAP_ADDR;
char *taproute = USTACK_TAP_ROUTE;

static int set_if_route(char *dev, char *cidr)
{
	char buf[256];

	sprintf(buf, "ip route add dev %s %s", dev, cidr);
	printf("[DEBUG] %s\n", buf);
	system(buf);
}

static int set_if_address(char *dev, char *cidr)
{
	char buf[256];

	sprintf(buf, "ip address add dev %s local %s", dev, cidr);
	printf("[DEBUG] %s\n", buf);
	system(buf);
}

static int set_if_up(char *dev)
{
	char buf[256];
/*	sprintf(buf, "ip link set %s address 00:11:22:33:44:55", dev);
	printf("[DEBUG] %s\n", buf);
	system(buf);
*/
	sprintf(buf, "ip link set dev %s up", dev);
	printf("[DEBUG] %s\n", buf);
	system(buf);
}

static int tun_alloc(char *dev)
{
	struct ifreq ifr;
	int fd, err, i, flags;
	struct ifreq s;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		printf("[FATAL] Cannot open TUN/TAP dev\nMake sure one exists with '$ mknod /dev/net/tun c 10 200'\n");
		exit(-1);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	if (*dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
		printf("[FATAL] Could not ioctl tun");
		close(fd);
		return err;
	}

	if ((err = ioctl(fd, SIOCGIFHWADDR, &s)) == 0) {
		memcpy(mymac, s.ifr_addr.sa_data, 6);
		printf("[DEBUG] tap interface configured\n");
	} else {
		printf("[FATAL] Could not get interface MAC address");
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
/*	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
*/
	return fd;
}

void tun_init()
{
	dev = calloc(10, 1);
	tun_fd = tun_alloc(dev);

	if (set_if_up(dev) != 0)
		printf("[FATAL] Setting up interface failed.\n");

	if (set_if_route(dev, taproute) != 0)
		printf("[FATAL] Setting route for interface failed\n");

	if (set_if_address(dev, tapaddr) != 0)
		printf("[FATAL] Setting address for interface failed\n");
}

void tun_deinit()
{
	free(dev);
}

void en_ll_output(uint8_t *frame, uint16_t size)
{
#ifdef USTACK_DEBUG_FRAMES
	hexdump(frame, size);
#endif
	write(tun_fd, frame, size);
}

int32_t en_ll_input(uint8_t *frame)
{
	int32_t size;
	
	size = read(tun_fd, frame, FRAME_SIZE);
#ifdef USTACK_DEBUG_FRAMES
	if (size > 0)
		hexdump(frame, size);
#endif
	return size;
}
