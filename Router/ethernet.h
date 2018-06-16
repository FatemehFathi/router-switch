#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/time.h>


using namespace std;


class Eth {
public:
    Eth() { portNum = "notSet"; empty = true; sockfd = -1; is_host = true; router_port = "-1"; cost = "1"; IP = "notSet"; IP_connect_to = "notSet"; }
    
	void set_portNum(string ethP) { portNum = ethP; }
	void set_router_port(string routerPort) { router_port = routerPort; }
	void set_empty(bool emp) { empty = emp; }
	void set_sockfd(int fd) { sockfd = fd; }
	void set_is_host(bool is_h) { is_host = is_h; }
	void set_cost (string c) { cost=c; }
	void set_IP(string i)  { IP=i; }
	void set_IP_connect_to(string i) { IP_connect_to=i; }
    
	string get_portNum() { return portNum; }
    string get_router_port() { return router_port; }
	bool get_empty() { return empty; }
    int get_sockfd() { return sockfd; }
    bool get_is_host() { return is_host; }
	string get_cost() { return cost; }
	string get_IP() { return IP; }
	string get_IP_connect_to() { return IP_connect_to; }
private:
	string portNum; //eth0
	string router_port; //port
	bool empty; //yani hosti o dare ya na !
	int sockfd;
	bool is_host;
	string cost;
	string IP;
	string IP_connect_to; //ip e hoste connect gardide
};


#endif

