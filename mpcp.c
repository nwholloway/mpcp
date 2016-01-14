#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include <stdio.h>


static struct ifreq *find_ifreq(int s, struct ifconf *iflist)
{
	int index;

	for (index = 0; index < iflist->ifc_len/sizeof(struct ifreq); index++)
	{
		struct ifreq *ifreq = &iflist->ifc_req[index];

		if (ifreq->ifr_addr.sa_family != AF_INET)
		{
			continue;
		}
		if (ioctl(s, SIOCGIFFLAGS, ifreq) == -1)
		{
			perror("siocgifflags");
			return NULL;
		}
		if ((ifreq->ifr_flags & (IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_LOOPBACK|IFF_POINTOPOINT)) == (IFF_UP|IFF_RUNNING|IFF_BROADCAST))
		{
			return ifreq;
		}
	}

	return NULL;
}

static int sockaddr_init(int s, struct sockaddr_ll *socket_address)
{
	unsigned char ifbuf[1024];
	struct ifconf iflist;
	struct ifreq *ifreq;

	iflist.ifc_buf = (char*) &ifbuf;
	iflist.ifc_len = sizeof(ifbuf);

	if (ioctl(s, SIOCGIFCONF, &iflist) == -1)
	{
		perror("siocgifconf");
		return -1;
	}

	ifreq = find_ifreq(s, &iflist);
	if (ifreq == NULL)
	{
		return -1;
	}

	if (ioctl(s, SIOCGIFINDEX, ifreq) == -1)
	{
		perror("siocgifindex");
		return -1;
	}

	socket_address->sll_family = PF_PACKET;
	socket_address->sll_protocol = htons(ETH_P_PAUSE);
	socket_address->sll_ifindex = ifreq->ifr_ifindex;
	socket_address->sll_hatype = ARPHRD_ETHER;
	socket_address->sll_pkttype = PACKET_OTHERHOST;

	if (ioctl(s, SIOCGIFHWADDR, ifreq) == -1)
	{
		perror("siocgifhwaddr");
		exit(1);
	}

	socket_address->sll_halen = ETH_ALEN;
	memcpy(socket_address->sll_addr, ifreq->ifr_hwaddr.sa_data, ETH_ALEN);

	return 0;
}

int main(int argc, char** argv)
{
	int s;
	unsigned char pause_multicast[ETH_ALEN] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x01 };
	unsigned char packet[ETH_ZLEN+ETH_FCS_LEN];
	struct ethhdr *eh = (struct ethhdr *)packet;
	char* payload;
	struct sockaddr_ll socket_address;

	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_PAUSE));
	if (s == -1)
	{
		perror("socket");
		exit(1);
	}

	if (sockaddr_init(s, &socket_address) < 0)
	{
	    exit(1);
	}

	memset(packet, 0, ETH_ZLEN+ETH_FCS_LEN);
	memcpy(eh->h_dest, pause_multicast, ETH_ALEN);
	memcpy(eh->h_source, socket_address.sll_addr, ETH_ALEN);
	eh->h_proto = socket_address.sll_protocol;

	payload = packet + sizeof(struct ethhdr);
	payload[0] = 0x00;
	payload[1] = 0x01;

	payload[2] = 0x06;
	payload[3] = 0x50;

	for (;;)
	{
		sendto(s, packet, 60, 0,
			(struct sockaddr*)&socket_address, sizeof(socket_address));
	}
}
