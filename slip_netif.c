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

FILE *tin, *tout;
int fd;

int32_t if_setup()
{
	if ((fd = open(SERIAL_DEV0, O_RDWR)) < 0) {
		printf("[FATAL] Cannot open serial interface %s\n", SERIAL_DEV0);
		
		return -1;
	}
/*	} else {
		tout = fopen(SERIAL_DEV0, "w");
		tin = fopen(SERIAL_DEV0, "r");
		if ((tout && tin)) {
			printf("[DEBUG] Serial interface %s initialized\n", SERIAL_DEV0);
		
			return 0;
		} else {
			printf("[FATAL] Error opening file descriptors\n");
			
			return -1;
			
		}
	}*/
}

static void tty_write(uint8_t byte)
{
	write(fd, &byte, 1);
}

static uint8_t tty_read(void)
{
	uint8_t byte;
	
	read(fd, &byte, 1);
	
	return byte;
}
 
uint16_t netif_send(uint8_t *packet, uint16_t len)
{
	uint16_t i;

	tty_write(SLIP_END);
	for (i = 0; i < len; i++) {
		if (packet[i] == SLIP_END) {
			tty_write(SLIP_ESC);
			tty_write(SLIP_ESC_END);
		} else {
			if (packet[i] == SLIP_ESC) {
				tty_write(SLIP_ESC);
				tty_write(SLIP_ESC_ESC);
			} else {
				tty_write(packet[i]);
			}
		}
	}
	tty_write(SLIP_END);
	
#ifdef USTACK_DEBUG_FRAMES
	hexdump(packet, len);
#endif
	
	return len;
}

uint16_t netif_recv(uint8_t *packet)
{
	uint16_t len = 0;
	int16_t r;

	while (1) {
		r = tty_read();
		if (r == SLIP_END) {
			if (len > 0) {
#ifdef USTACK_DEBUG_FRAMES
				hexdump(packet, len);
#endif
				return len;
			}
		} else {
			if (r == SLIP_ESC) {
				r = tty_read();
				if (r == SLIP_ESC_END) {
					r = SLIP_END;
				} else {
					if (r == SLIP_ESC_ESC) {
						r = SLIP_ESC;
					}
				}
			}
			packet[len++] = r;
			if (len > FRAME_SIZE)
				len = 0;
		}
	}
}
