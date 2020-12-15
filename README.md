# uStack - A portable and minimalistic IP stack

uStack is a quick-and-dirty implementation of most common network protocols of IPv4, suitable for experiments and integration with embedded devices with severe resource constraints. Currently, uStack supports a low level abstraction of the network card (using Linux TUN/TAP), Ethernet, ARP, BOOTP, IP, ICMP and UDP protocols.

The stack is easily portable to different platforms, such as AVR or STM32 microcontrollers. The TUN/TAP driver can be replaced by real Ethernet MACs/PHYs such as the ENC28J60 chip, along with software to initialize the device and handle raw Ethernet frames. Other alternatives include the use of a basic SLIP driver.

## Configuration

Just type *make* to build the binary. Run it with root permissions using *sudo ./ustack*. A TAP interface will be created with a random MAC address.

## Demo

The demo application (main.c) is configured with almost all debug flags enabled by default (configured in the *makefile*). It defines a static IP address / network mask a UDP callback routine and an application loop which checks for received packets and injects them in the stack. To reach the application try this:

    $ ping -c 3 172.31.69.20

    PING 172.31.69.20 (172.31.69.20) 56(84) bytes of data.
    64 bytes from 172.31.69.20: icmp_seq=1 ttl=64 time=0.160 ms
    64 bytes from 172.31.69.20: icmp_seq=2 ttl=64 time=0.135 ms
    64 bytes from 172.31.69.20: icmp_seq=3 ttl=64 time=0.139 ms

    --- 172.31.69.20 ping statistics ---
    3 packets transmitted, 3 received, 0% packet loss, time 2030ms
    rtt min/avg/max/mdev = 0.135/0.144/0.160/0.017 ms

Then, test the *echo protocol* using UDP on port 7  or the demo application on port 30168:

    $ echo "hi there" | nc -u -w1 172.31.69.20 7
    hi there

If you want define an IP dinamically, change the default IP, network mask and gateway addresses to zeroes in the application and rebuild the binary. Then, add a static ARP entry in your system defining both IP and MAC addresses (the MAC address is presented in the debug messages) and use *ping* with a size of 113 to configure the device:

    $ sudo arp -a 172.31.69.30 -s ee:0d:78:ce:7d:4a

    $ ping -c 3 -s 113 172.31.69.30

    PING 172.31.69.30 (172.31.69.30) 113(141) bytes of data.
    121 bytes from 172.31.69.30: icmp_seq=2 ttl=64 time=0.106 ms
    121 bytes from 172.31.69.30: icmp_seq=3 ttl=64 time=0.106 ms

    --- 172.31.69.30 ping statistics ---
    3 packets transmitted, 2 received, 33% packet loss, time 2054ms
    rtt min/avg/max/mdev = 0.106/0.106/0.106/0.000 ms

The first ping is lost, because it is used to configure the network address.
