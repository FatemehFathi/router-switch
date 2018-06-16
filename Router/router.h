#ifndef __ROUTER_H__
#define __ROUTER_H__

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
#include <utility>
#include <arpa/inet.h>

#include "ethernet.h"
#include "distVectRow.h"


using namespace std;


#define STDIN 0
#define TRUE 1
#define MESSAGE_SIZE 1024
#define BUFF_SIZE 1024
#define MAX_CLIENT 100



class Router {
public:
	void run(const char* port);
	int  parse (string s, int sockfd, fd_set readfds);
	void set_listen_port(string LP) { listen_port = LP; }
	
	void create_ethernet(string ethCard_num);
	void delete_ethernet(string ethNum, fd_set readfds);
	void setIP(string ethCard_num,string IP);
	void delete_IP(string ethNum);
	int  connect_router(string myeth, string peer_eth, string peer_port);
	void setRouterLink(string my_eth, string peer_port, int sockfd);
	void change_cost(string ethCard_num, string cost);
	void disconnect_eth(string ethCard_num, fd_set readfds);
	void connect_host(string port_num,string ip_addr,int sockfd);
	void disconnect_request(int sockfd, fd_set readfds);
	void print_ethernets();
    
    //routing table
    void update_routing_table(string dest, string eth, string cost, int sockfd, bool cost_changed);
    void deleteFrom_routing_table(string dest, int sockfd);
    void deleteRouter_routing_table(string eth, int sockfd);
    void myRoutingTable_for1(int sockfd, string rt);
    void myRoutingTable_for2(int sockfd, string rt);
    void print_routing_table();
    
    string find_eth_of_sockfd(int sockfd);
    int find_sockfd_of_eth(string eth);
    bool eth_is4host(string eth);
    
    //ping
    void ping(int sockfd, string ping_ip, string ip_addr);
    void response2ping(int sockfd, string ip1, string ping_ip);
    void found_ping(int sockfd, string ip1, string ping_ip);
    
    //trace
    void 	Trace ( string packet );
	bool 	ifSame ( string packet );
	void 	turnBackPacket(string packet);
	int 	getSockfdByPort(string port);
	void 	sendNextRouter(string packet,string ethNum);
	int 	getSockfd(string ethNum);
	string 	ip (string packet);
	int 	isConnected(string packet,string &ethNum) ;
	bool 	host_interface(string interface);
	string 	makeTracePacket(string msg,int fd);
	void 	backingTrace(string packet);
	int 	getSockfdPacket(string packet);
	string 	portFromPacket(string newPacket);
	string 	plusPacket(string newPacket);
	bool 	checkLastRouter(string packet);
	
	string 	reverse(string newPacket);
    
private:
    string listen_port;
    vector<Eth*> ethernets;
    vector<DistVectRow*> routing_table;
};



#endif

