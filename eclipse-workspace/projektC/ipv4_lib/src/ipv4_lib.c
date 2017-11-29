/*
 * ipv4_lib.c
 *
 *      Author: janek
 */

#include <netinet/ip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<netdb.h> //hostent
#define BUFFSIZE 4096

//function to calculate checksum of ipv4 packet
unsigned short Checksum(unsigned short *datagram, int nbytes) {
	//

	long sum;
	short answer;

	sum = 0;

	while (nbytes > 1) {
		sum += *datagram++;
		nbytes -= 2;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short) ~sum;
	return answer;
}

//function to create ipv4 packet
unsigned short * Ipv4Packet() {

	char *data;
	int i;
	char *datagram = malloc( BUFFSIZE);

	memset(datagram, 0, 4096);	//clear datagram memory

	struct iphdr *iph = (struct iphdr *) datagram;	//IP header
	struct in_addr addr;

	data = (char *) datagram + sizeof(struct iphdr);

	char instr[20];	//input buffer

	//fill header fields
	printf("Tworzenie pakietu IPv4 (jeśli puste, domyślnie): \n");
	printf("Dane  (ABCDEFGHIJKLMNOPQRSTUVWXYZ): ");
	fgets(instr, 20, stdin);
	i = strlen(instr) - 1;
	if (instr[i] == '\n')
		instr[i] = '\0';

	if ((strcmp(instr, "") == 0))
		strcpy(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ)");
	else
		strncpy(data, instr, strlen(instr));

	printf("Długość nagłówka  (5): ");
	fgets(instr, 20, stdin);

	if (atoi(instr) == 0)
		iph->ihl = 5;	//internet header length
	else
		iph->ihl = atoi(instr);

	printf("Wersja IP: 4. \n");
	iph->version = 4;	//version 4

	printf("Typ usługi (20): ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		iph->tos = 20;
	else
		iph->tos = atoi(instr);

	printf("Całkowita długość pakietu zostanie wyliczona \n");
	iph->tot_len = sizeof(struct iphdr) + strlen(data);	//total length

	printf("Numer identyfikacyjny (40110): ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		iph->id = htons(40110);	//id
	else
		iph->id = htons(atoi(instr));

	printf("Przesunięcie (0): ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		iph->frag_off = 0;
	else
		iph->frag_off = atoi(instr);

	printf("Czas życia (64): ");
	fgets(instr, 20, stdin);
	if (atoi(instr) == 0)
		iph->ttl = 64;
	else
		iph->ttl = atoi(instr);

	printf("Protokół warstwy wyższej - IPPROTORAW \n");
	iph->protocol = IPPROTO_RAW;

	printf("Suma kontrolna nagłówka zostanie wyliczona...\n");
	iph->check = 0;		//Set to 0 before calculating checksum

	printf("Adres źrodłowy (192.168.1.168): ");
	fgets(instr, 20, stdin);

	iph->saddr = inet_addr(instr);
	if (atoi(instr) == 0)
		iph->saddr = inet_addr("192.168.1.168");	//source address
	else {

		i = strlen(instr) - 1;
		if (instr[i] == '\n')
			instr[i] = '\0';

		while (inet_pton(AF_INET, instr, &addr) != 1) {
			fprintf(stderr, "Invalid address\n");
			fgets(instr, 20, stdin);
			i = strlen(instr) - 1;
			if (instr[i] == '\n')
				instr[i] = '\0';
		}

	}

	printf("Adres docelowy (5.134.213.80): ");

	fgets(instr, 20, stdin);
	i = strlen(instr) - 1;
	if (instr[i] == '\n')
		instr[i] = '\0';
	if (atoi(instr) == 0 && (strcmp(instr, "") == 0))
		iph->daddr = inet_addr("5.134.213.80");	//destination address
	else {
		i = strlen(instr) - 1;
		if (instr[i] == '\n')
			instr[i] = '\0';

		struct hostent *he;
		struct in_addr **addr_list;

		he = gethostbyname(instr);
		while (!he) {
			fgets(instr, 20, stdin);
			i = strlen(instr) - 1;
			if (instr[i] == '\n')
				instr[i] = '\0';
			he = gethostbyname(instr);
		}
		addr_list = (struct in_addr **) he->h_addr_list;
		char * zmienna = inet_ntoa(*addr_list[0]);

		iph->daddr = inet_addr(zmienna);

	}

	//printf("Data automaticly set: %s\n", data);	//data

	iph->check = Checksum((unsigned short *) datagram, iph->tot_len);

	return (unsigned short *) datagram;

}

