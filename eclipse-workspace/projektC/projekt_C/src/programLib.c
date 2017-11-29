#include <dlfcn.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <net/if.h>
#include "programLib.h"
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <stdbool.h>

struct dnode *start = NULL;
int usleep(useconds_t useconds);

void AddToList(int *counter, char *datagram) {
	for (int i = 0; i != *counter; i++) {
		struct dnode *nptr;

		/* create a new node */
		nptr = malloc(sizeof(struct dnode));

		/* assign data and pointer to the new node */
		nptr->prev = NULL;
		nptr->datagram = datagram;
		nptr->next = start;

		if (start != NULL)
			start->prev = nptr;
		start = nptr;
	}
}
void DisplayList() {
	struct dnode *temp = start;
	printf("\n");
	if (temp == NULL)
		printf("\nLista jest pusta\n");
	int i = 0;

	/* traverse the entire linked list */
	while (temp != NULL) {
		i++;
		printf("id:%d, datagram: %p\n", i, (void *) (temp->datagram));
		temp = temp->next;

	}


}
void DeleteList() {

	struct dnode *temp = start;
	struct dnode *del = NULL;

	while (temp != NULL) {
		del = temp;
		temp = temp->next;
		free(del);
	}
	start = NULL;

}


//function to send packets from linked list
int SendPacket(char *interface) {

	char *datagram = start->datagram;
	struct iphdr *iph = (struct iphdr *) datagram;

	//open socket
	int s ;
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			perror("socket() failed to get socket descriptor for using ioctl() ");
			exit(EXIT_FAILURE);
		}

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = iph->daddr;

	struct ifreq ifr;	//struct to take name of interface

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {		//find typed interface
		perror("ioctl() failed to find interface ");
		return (EXIT_FAILURE);
	}

	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (void *) &ifr, sizeof(ifr))
			< 0) {	//bind socket to interface
		printf("\nNot binded\n");
		return EXIT_FAILURE;
	}

	while (start != NULL) {	//go to end of list

		if (sendto(s, datagram, iph->tot_len, 0, (struct sockaddr *) &sin,
				sizeof(sin)) < 0) {	//send packet
			perror("Sendto failed: ");
			return EXIT_FAILURE;
		} else
			printf("Wysłano pakiet o długości: %d \n", iph->tot_len);

		start = start->next;	//go to next node
		usleep(100000);

	}
	close(s);	//close socket
	DeleteList();
	DisplayList();
	return EXIT_SUCCESS;
}


int Window(int *counter, char *interface) {

	char instr[20];	//data from input stream
	int operation;
	int i;
	printf("\e[1;1H\e[2J");
	printf("Wyślij pakiet IPv4 - domyślnie \n");
	printf("(2) Wyślij pakiet UDP \n");

	fgets(instr, 20, stdin);	//get number of operation
	operation = atoi(instr);

	printf("\e[1;1H\e[2J");
	printf("Type name of interface you want to use.\n");

	PrintInterfaces();
	printf("Nazwa (domyślnie \"lo\"):");
	fgets(instr, 20, stdin);	//get name of interface
	i = strlen(instr) - 1;
	if (instr[i] == '\n')
		instr[i] = '\0';
	if (atoi(instr) == 0 && (strcmp(instr, "") == 0))
		memcpy(interface, "lo", 4);
	else
		memcpy(interface, instr, strlen(instr));

	printf("Podaj liczbę pakietów jaką chcesz wysłać (domyślnie \"5\") ");
	fgets(instr, 20, stdin);	//get number of packets to send
	if (atoi(instr) == 0)
		*counter = 5;
	else
		*counter = atoi(instr);

	return operation;
}

void PrintInterfaces() {
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
		NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if ( /*(strcmp(ifa->ifa_name,"wlan0")==0)&&( */ifa->ifa_addr->sa_family
				== AF_INET) // )
		{
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			printf("\tInterface : <%s>\n", ifa->ifa_name);
			printf("\t  Address : <%s>\n", host);
		}
	}

	freeifaddrs(ifaddr);
}
