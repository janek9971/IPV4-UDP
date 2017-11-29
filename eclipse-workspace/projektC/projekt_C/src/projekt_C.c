#include <stdio.h>
#include <stdlib.h>
#include "programLib.h"
#include <dlfcn.h>
#include <netinet/in.h>
#include <error.h>

int main(void) {

	void * Ipv4Library;
	void * UdpLibrary;

	int *counter;
	char *interface;
	unsigned short *datagram;
	unsigned short * (*ipv4)() ;
	void (*udp)(unsigned char *) ;

	datagram = malloc(sizeof(4096));
	counter = malloc(sizeof(int));
	interface = malloc(sizeof(char) * 32);

	int a = Window(counter, interface);

	Ipv4Library = dlopen("./ipv4_lib.so", RTLD_LAZY);
	if (!Ipv4Library) {
		fputs(dlerror(), stderr);
		exit(1);
	}

	ipv4 = dlsym(Ipv4Library, "Ipv4Packet");
	datagram = (*ipv4)();

	if (a == 2) {

		UdpLibrary = dlopen("./udp_lib.so", RTLD_LAZY);
		if (!UdpLibrary) {
			fputs(dlerror(), stderr);
			exit(1);
		}
		udp = dlsym(UdpLibrary, "UdpPacket");

		(*udp) ((unsigned char *) datagram);
	}

	AddToList(counter, (char*) datagram);
	DisplayList();

	SendPacket(interface);
	EXIT_SUCCESS;
}
