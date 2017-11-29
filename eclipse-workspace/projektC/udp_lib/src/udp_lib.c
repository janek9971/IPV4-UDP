#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/udp.h>

struct pseudo_header {
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};

unsigned short csum(unsigned short *ptr, int nbytes) {
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		*((u_char*) &oddbyte) = *(u_char*) ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short) ~sum;

	return (answer);
}

void UdpPacket(unsigned char * datagram) {

	int i;
	struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct iphdr));
	struct iphdr *iph = (struct iphdr *) datagram;
	char * udpdata = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
	struct pseudo_header *pshd = malloc(sizeof(struct pseudo_header));
	char * pseudogram;
	char instr[20];

	iph->protocol = IPPROTO_UDP;
	memset(udph, 0, iph->tot_len);
	printf("\nUDP dane: (ABCDEFGHIJKLMNOPQRSTUVWXYZ)) ");
	fgets(instr, 20, stdin);
	i = strlen(instr) - 1;
	if (instr[i] == '\n')
		instr[i] = '\0';

	if ((strcmp(instr, "") == 0))
		strcpy(udpdata, "ABCDEFGHIJKLMNOPQRSTUVWXYZ)");
	else
		strncpy(udpdata, instr, strlen(instr));

	iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr)
			+ strlen(udpdata);

	printf("\nUDP port źródłowy (3333) ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		udph->source = htons(3333);
	else
		udph->source = htons(atoi(instr));

	printf("\nUDP port docelowy: (6666) ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		udph->dest = htons(6666);
	else
		udph->dest = htons(atoi(instr));

	udph->len = htons(sizeof(struct udphdr) + strlen(udpdata));
	udph->check = 0;

	pshd->source_address = iph->saddr;
	pshd->dest_address = iph->daddr;
	pshd->placeholder = 0;
	pshd->protocol = IPPROTO_UDP;
	pshd->udp_length = udph->len;

	int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr)
			+ strlen(udpdata);
	pseudogram = malloc(psize);

	memcpy(pseudogram, (char*) pshd, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), udph,
			sizeof(struct udphdr) + strlen(udpdata));

	udph->check = csum((unsigned short*) pseudogram, psize);
	//printf ("\nCheck: %X", udph->check);
}
