#ifndef __HOST_H__
#define __HOST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>

using namespace std;

#define MESSAGE_SIZE 1024

void showTrace(string packet);
string reverse(string newPacket);

class Host {
public:
    Host() { eth = "notSet"; ip_addr = "notSet"; connected_router = "notSet"; }

	void set_ip_addr(string ip) { ip_addr = ip; }
	void set_eth(string e) { eth = e; }
	void connect_router(string listen_port) { connected_router = listen_port; }
	void disconnect_router() { connected_router = "notSet"; }
    void set_sockfd(string sfd) { sockfd = sfd; }
    
   	string get_eth(){ return eth; }
	string get_ip_addr() { return ip_addr; }
	string get_connected_router() { return connected_router; }
    string get_sockfd() { return sockfd; }
    
private:
    string ip_addr;
    string eth;	// oon etherneti k too oon router behesh motasel shode !
    string connected_router;
    string sockfd;
};
#endif
