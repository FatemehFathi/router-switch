#include "router.h"



int main(int argc, char* argv[]) {
    Router* s = new Router();
    
    string input;
    bool set_port = false;
    string port;
    
    while (!set_port) {
        cin >> input;
        if (input == "#ListenPort") {
            cin >> port;
            set_port = true;
            s->set_listen_port(port);
        }
        else
            cout << "set #ListenPort first please" << endl << endl;
    }
    
    s->run(port.c_str());
    return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////////

void Router::run(const char* port)
{
	int clientFds[MAX_CLIENT];
	int clientIndex=0;
	
	fd_set readfds;
    
	char recvBuff[BUFF_SIZE];
	char sendBuff[1025];
	memset(sendBuff, '0', sizeof(sendBuff));
    
	struct sockaddr_in serv_addr,client;
	memset(&serv_addr, '0', sizeof(serv_addr));
	int p = atoi(port);
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(p);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);	//socket(PF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
        cerr << "error: socket()" << endl;
        exit(1);
	}
    
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
	if(listen(listenfd, 10) == -1) {
        cerr << "Failed to listen" << endl;
		exit(1);
	}
	
	
	int newSock, n;
	socklen_t clientSize = sizeof(client);
    clientFds[0]=0;
    clientIndex=1;
    while(1) {
        struct timeval timeout;
        timeout.tv_sec = 3;     /* every 3 seconds */
        timeout.tv_usec = 400;  /* plus 400 msec */
        FD_ZERO(&readfds);
        FD_SET (listenfd, &readfds);
        FD_SET (0,&readfds);
        for (int d = 0; d < clientIndex; d++) {
            FD_SET(clientFds[d], &readfds);
        }
        
        int q = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
        if (FD_ISSET(listenfd, &readfds)) {
            newSock = accept(listenfd, (struct sockaddr *)&client, (socklen_t*)&clientSize);
            clientFds[clientIndex] = newSock;
            clientIndex++;
        }
        
        for(int c = 0; c < clientIndex; c++ ) {
            int fd = clientFds[c];
            if (FD_ISSET(fd, &readfds)) {
                if(fd == 0) {
                    memset(recvBuff, 0, BUFSIZ);
                    int R0=read(0,recvBuff,BUFSIZ);
                    //cout<<"your message: "<<recvBuff<<"*";
                    string msg(recvBuff);
                    msg += "<!!!>";
                    int fromParse=parse(msg, fd, readfds);
                    if(fromParse>1)	// yani man b ye router vasl shodam !
                    {	//cout<<"fromParse "<<fromParse<<endl;
                        clientFds[clientIndex] = fromParse;
                        clientIndex++;
                    }
                    /*
                    cout<<"client ha --> ";
                    for(int d=0;d<clientIndex;d++)
                        cout<<clientFds[d]<<" ";
                    cout<<endl;
                     */
                }
                else {
                    n = recv(fd, recvBuff, BUFF_SIZE - 1, 0);
                    
                    if(n < 0){
                        cerr << "error: recv()" << endl;
                    } else if(n > 0) {
                        recvBuff[n] = '\0';
                        cout << "Rcved msg from fd = " << fd << ": " << recvBuff << endl << endl;
                        string msg(recvBuff);
                        //int alaki;
                        int pp;
                        if (pp = msg.find("#Trace") != string::npos)
                            msg=makeTracePacket(msg,fd);

                        parse(msg, fd, readfds);
                    } else {
                        cout << "faz chie n=0 on recv :P " << endl;
                    }
                }
            }
        }
        //cout<<endl;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router::parse(string s, int sockfd, fd_set readfds) {
	int q;
	int p;
    
    if (p = s.find("#Eth") != string::npos) { //#Eth eth0
		q = s.find("<!!!>");
		string ethCard_num = s.substr(p + 4, q - p - 5);
        
        create_ethernet(ethCard_num);
	}
	else if (p = s.find("#NoEth") != string::npos) { //#NoEth eth0
		q = s.find("<!!!>");
		string ethCard_num = s.substr(p + 6, q - p - 7);
        
        delete_ethernet(ethCard_num, readfds);
	}
	else if (p = s.find("#IP") != string::npos) { //#IP eth0 10.10.10.3
		s.erase(0 , p + 3);
		q = s.find(" ");
		string ethCard_num = s.substr(0, q);
		p = s.find(" ");
		q = s.find("<!!!>");
		string ip_addr = s.substr(p + 1, q - p - 2);
        
		setIP(ethCard_num, ip_addr);
	}
	else if (p = s.find("#NoIP") != string::npos) {	//#NoIP eth0
		q = s.find("<!!!>");
		string ethCard_num = s.substr(p + 5, q - p - 6);
        
		delete_IP(ethCard_num);
	}
	else if (p = s.find("#Connect") != string::npos) { //#Connect eth0 eth3 2024
		s.erase(0, p + 8);
		q = s.find(" ");
		string my_eth = s.substr(0, q);
		s.erase(0, q + 1);
		q = s.find(" ");
		string peer_eth = s.substr(0, q);
		s.erase(0, q + 1);
		q = s.find("<!!!>");
		string peer_port = s.substr(0, q - 1);
        
		int fdConnect=connect_router(my_eth, peer_eth, peer_port);
		return fdConnect;
	}
    else if (p = s.find("#ChangeCost") != string::npos) { //#ChangeCost eth0 10
		s.erase(0 , p + 11);
		q = s.find(" ");
		string ethCard_num = s.substr(0, q);
		p = s.find(" ");
		q = s.find("<!!!>");
		string cost = s.substr(p + 1, q - p - 2);
        
		change_cost(ethCard_num, cost);
	}
    else if (p = s.find("#Disconnect") != string::npos) { //#Disconnect eth0
		q = s.find("<!!!>");
		string ethCard_num = s.substr(p + 11, q - p - 12);

		disconnect_eth(ethCard_num, readfds);
	}
    else if (p = s.find("#Show") != string::npos) {
		print_routing_table();
	}
    
    else if (p = s.find("#PrintEthernets") != string::npos) {
		print_ethernets();
	}
    
    
    //////////////////////////////////////////////////////////
    
    
    
	//"<connectRouter>" + peer_eth + "</connectRouter>" + listen_port + "</peerPort>";
	else if (p = s.find("<connectRouter>") != string::npos) { // ye router khaste be man connect she
		q = s.find("</connectRouter>");
		string my_eth = s.substr(p + 14, q - p - 14);
		p = s.find("</connectRouter>");
		q = s.find("</peerPort>");
        string peer_port = s.substr(p + 15 + 1, q - p - 15 -1);
		
		setRouterLink(my_eth, peer_port, sockfd);
	}
	
    
    //"<RoutingTable>" + updated->get_destination() + "</dest>" + updated->get_next_hop() + "</nextHop>" + updated->get_cost() + "</cost>";
    else if (p = s.find("<broadcastRT>") != string::npos) { //broadcast routing table
        q = s.find("</dest>");
		string dest = s.substr(p + 12, q - p - 12);
		p = s.find("</dest>");
		q = s.find("</nextHop>");
		string next_hop = s.substr(p + 7, q - p - 7);
		p = s.find("</nextHop>");
		q = s.find("</cost>");
		string cost = s.substr(p + 10, q - p - 10);
        p = s.find("</cost>");
		q = s.find("</costChanged>");
		string cost_changed = s.substr(p + 7, q - p - 7);
        
        bool costChanged = false;
        if (cost_changed == "1") costChanged = true;
		
		update_routing_table(dest, next_hop, cost, sockfd, costChanged);
	}
    
    else if (p = s.find("<myRoutingTable1>") != string::npos) {
		myRoutingTable_for1(sockfd, s);
	}
    
    else if (p = s.find("<myRoutingTable2>") != string::npos) {
		myRoutingTable_for2(sockfd, s);
	}
    
    else if (p = s.find("<deleteFromRT>") != string::npos) {
		q = s.find("</dest>");
        string dest = s.substr(p + 13, q - p - 13);
        
        deleteFrom_routing_table(dest, sockfd);
	}
    
	// msg i k oomade az host ---->  "<Gateway>" + h->get_eth() + "</eth>" + h->get_ip_addr() + "</ip>";
	else if (p = s.find("<Gateway>") != string::npos) { //#Gateway 2021 eth0
		q = s.find("</eth>");
		string eth_num = s.substr(p + 8, q - p - 8);
		p = s.find("</eth>");
		q = s.find("</ip>");
		string ip_addr = s.substr(p + 6, q - p - 6);
        
		connect_host(eth_num, ip_addr, sockfd);
	}
    
    
	else if (p = s.find("<DC>") != string::npos) { //darkhaste dc az tarafe ye host ya ye router !!
		disconnect_request(sockfd, readfds);
	}
    
    
    ////////////////////////////////////////////////
    
    
	else if (p = s.find("<ping>") != string::npos) { //#Ping 10.10.10.4
        q = s.find("</ping>");
		string ping_ip = s.substr(p + 5, q - p - 5);
		p = s.find("</ping>");
		q = s.find("</ip>");
		string ip_addr = s.substr(p + 7, q - p - 7);
        
        ping(sockfd, ping_ip, ip_addr);
	}
    
    else if (p = s.find("<response2ping>") != string::npos) { //resonse2ping()
        q = s.find("</ip1>");
		string ip1 = s.substr(p + 14, q - p - 14);
		p = s.find("</ip1>");
		q = s.find("</pingIP>");
        string ping_ip = s.substr(p + 6, q - p - 6);
        
        response2ping(sockfd, ip1, ping_ip);
	}
    
    else if (p = s.find("<foundPing>") != string::npos) { //found_ping()
        q = s.find("</ip1>");
		string ip1 = s.substr(p + 10, q - p - 10);
		p = s.find("</ip1>");
		q = s.find("</pingIP>");
        string ping_ip = s.substr(p + 6, q - p - 6);
        
        found_ping(sockfd, ip1, ping_ip);
	}
    
    else if (p = s.find("<Trace>0") != string::npos) { // yani trace ip , ip peida nashode hanooz - dare mire jolo packet
        q = s.find(":D");
        string ip = s.substr(p + 7 + 1, q - p - 7 - 1);
        
        Trace(s);
    }
    
    else if (p = s.find("<Trace>1") != string::npos) { // yani trace ip , ip peida shode , packet dare barmigarde !
        backingTrace(s);
    }
    
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::create_ethernet(string ethNum) { //#Eth eth0

	for(int i = 0; i < ethernets.size(); i++) {
		if(ethernets[i]->get_portNum() == ethNum) {
            cout << ethNum << " is already exist" << endl << endl;
            return;
        }
	}
    
	Eth* newEth = new Eth();
	newEth->set_portNum(ethNum);
	newEth->set_cost("1");
	ethernets.push_back(newEth);
    
	print_ethernets();
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::delete_ethernet(string ethNum, fd_set readfds) //#NoEth eth0
{
	bool found = false;
    bool is_host = false;
    string dest = "";
    int sockfd;
    
	for(int i = 0; i < ethernets.size(); i++) {
		if(ethernets[i]->get_portNum() == ethNum) {
            sockfd = ethernets[i]->get_sockfd();
            is_host = ethernets[i]->get_is_host();
            dest = ethernets[i]->get_IP_connect_to();
			ethernets.erase(ethernets.begin() + i);
			found = true;
		}
	}
    
	if(!found)
		cout << "Sorry! But " << ethNum << " doesn't exist !!" << endl << endl;
	else {
        FD_CLR(sockfd, &readfds);
        print_ethernets();
        
        if (is_host) {
            deleteFrom_routing_table(dest, 0);
            string to_send = "<connectError>";
            send(sockfd, to_send.c_str(), strlen(to_send.c_str()), 0);
        } else {
            deleteRouter_routing_table(ethNum, 0);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::setIP(string ethNum, string IP) { //IP EthernetCard IP_Address (#IP eth0 10.10.10.3)
    
	bool found = false;
	for(int i = 0; i < ethernets.size(); i++) {
		if(ethernets[i]->get_portNum() == ethNum) {
			ethernets[i]->set_IP(IP);
			found = true;
		}
	}
	if(!found)
		cout << "Sorry! But " << ethNum << " doesn't exist !!" << endl << endl;
	
    print_ethernets();
}

/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::delete_IP(string ethNum) { //NoIP EthernetCard (#NoIP eth0)
    
	bool found = false;
	for(int i = 0; i < ethernets.size(); i++) {
		if(ethernets[i]->get_portNum() == ethNum) {
			found = true;
			ethernets[i]->set_IP("notSet");
		}
	}
    
	if(!found)
		cout << "Sorry! But " << ethNum << " doesn't exist !!" << endl << endl;
	
    print_ethernets();
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router::connect_router(string myeth, string peer_eth, string peer_port)
{
	cout<<"connect to router "<<myeth<<"-"<<listen_port<<" ** "<<peer_eth<< "-" <<peer_port <<endl;
    string to_send = "<connectRouter>" + peer_eth + "</connectRouter>" + listen_port + "</peerPort>";

    // inja shabihe ye client amal mikone !
    struct sockaddr_in serv_addr;
    int sockfdC ;
    if( (sockfdC = socket(AF_INET,SOCK_STREAM,0) ) < 0 )
    {
        printf ("\n Error : Could not create socket \n");
        // return -1;
    }
    //cout<<"ba in fd vasl misham be oon switch ! : "<<sockfdC<<endl;
    //my_eth trunk shode , va port i k dare behesh goosh mide set shode ! va fd i k ba oon connecte
    setRouterLink(myeth,peer_port,sockfdC);
    
    char recvBuff[1024];
    memset(recvBuff, '0',sizeof(recvBuff));
    int peer_portINT=atoi(peer_port.c_str());
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(peer_portINT);
    
    if( connect(sockfdC, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        //return -1;
    }
    send(sockfdC, to_send.c_str(), strlen(to_send.c_str()), 0) ;
	
	print_ethernets();
	return sockfdC;
}



/////////////////////////////////////////////////////////////////////////////////////////////////


//ye routeri mikhad connect she bemoon.. age etherneti ke mikhasto dashtim connect mishim besho jadvalemuno mifrestim !

void Router::setRouterLink(string my_eth, string peer_port, int sockfdC) {
    
	bool found = false;
	for(int i = 0; i < ethernets.size(); i++) {
        if(ethernets[i]->get_portNum() == my_eth) {
            found = true;
            ethernets[i]->set_router_port(peer_port);
            ethernets[i]->set_empty(false);
            ethernets[i]->set_is_host(false);
            ethernets[i]->set_sockfd(sockfdC);
        }
	}
	
    
	if(!found)
		cout << my_eth << "nadaram ! famidiii ?!?!? :P " << endl << endl;
    else {
        //routing table ro mifreste !
        string rt = "<myRoutingTable1>";
        for (int i = 0; i < routing_table.size(); i++) {
            rt += routing_table[i]->get_destination() + "</dest>" + routing_table[i]->get_next_hop() + "</nextHop>" + routing_table[i]->get_cost() + "</cost>";
        }
        rt += "</myRoutingTable1>";
        
        for (int i = 0 ; i < ethernets.size(); i++) {
            if (!ethernets[i]->get_is_host()) {
                int sfd = ethernets[i]->get_sockfd();
                send(sfd, rt.c_str(), strlen(rt.c_str()), 0);
            }
        }
    }
    
    print_ethernets();
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////


//routeri ke besh darkhaste connect dade budim tunest connectemoon kone !! jadvalesham ferestad
//jadvalemuno update mikonimo broadcast mikonim be hameye routera ke bara unam bere :D
void Router::myRoutingTable_for1(int sockfd, string s) { // bara uni ke #Connecto zade
    //cout << "in MY ROUTING TABLE ooni ke connecto zade" << endl;
    
    vector<DistVectRow*> updated;
    
    int p, q;
    p = s.find("<myRoutingTable1>");
    s.erase(0, p + 17);
    
    while (p != string::npos) {
        q = s.find("</dest>");
        string dest = s.substr(0, q);
        s.erase(0, q + 7);
        q = s.find("</nextHop>");
        string next_hop = s.substr(0, q);
        s.erase(0, q + 10);
        q = s.find("</cost>");
        string cost = s.substr(0, q);
        s.erase(0, q + 7);
        
        p = s.find("</dest>");
        
        DistVectRow* row = new DistVectRow(dest, next_hop);
        row->set_cost(cost);
        updated.push_back(row);
    }
    
    
    //uni ke #Connecto zade jadvalesho update kone
    for (int i = 0; i < updated.size(); i++) {
        update_routing_table(updated[i]->get_destination(), updated[i]->get_next_hop(), updated[i]->get_cost(), sockfd, false);
    }
    
    
    //jadvale update shodasho befreste bara uni ke connectemun karde bude :p
    string rt = "<myRoutingTable2>";
    for (int i = 0; i < routing_table.size(); i++) {
        rt += routing_table[i]->get_destination() + "</dest>" + routing_table[i]->get_next_hop() + "</nextHop>" + routing_table[i]->get_cost() + "</cost>";
    }
    rt += "</myRoutingTable2>";
    
    send(sockfd, rt.c_str(), strlen(rt.c_str()), 0);
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////


//uni ke connect shode bud bemun jadvalesho update kardo aramun ferestad :D
void Router::myRoutingTable_for2(int sockfd, string s) { // bara uni ke #Connect shode behesh
    //cout << "in MY ROUTING TABLE ooni ke connect shode behesh" << endl;
    
    vector<DistVectRow*> updated;
    
    int p, q;
    p = s.find("<myRoutingTable2>");
    s.erase(0, p + 17);
    
    while (p != string::npos) {
        q = s.find("</dest>");
        string dest = s.substr(0, q);
        s.erase(0, q + 7);
        q = s.find("</nextHop>");
        string next_hop = s.substr(0, q);
        s.erase(0, q + 10);
        q = s.find("</cost>");
        string cost = s.substr(0, q);
        s.erase(0, q + 7);
        
        p = s.find("</dest>");
        
        DistVectRow* row = new DistVectRow(dest, next_hop);
        row->set_cost(cost);
        updated.push_back(row);
    }

    
    for (int i = 0; i < updated.size(); i++) {
        update_routing_table(updated[i]->get_destination(), updated[i]->get_next_hop(), updated[i]->get_cost(), sockfd, false);
    }
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::change_cost(string ethNum, string cost) { //ChangeCost EthernetCard NewCost (#ChangeCost eth0 10)

	bool found = false;
    Eth* eth = new Eth();
	for(int i = 0; i < ethernets.size(); i++) {
		if(ethernets[i]->get_portNum() == ethNum) {
			found = true;
			ethernets[i]->set_cost(cost);
            eth = ethernets[i];
		}
	}
    
	if(!found)
		cout << "Sorry! But " << ethNum << " doesn't exist !!" << endl << endl;
	else {
        print_ethernets();
        update_routing_table(eth->get_IP_connect_to(), ethNum, cost, -1000, true);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::connect_host(string port_num, string ip_addr, int sockfd) {
    bool well_done = false;
    
    for(int i = 0; i < ethernets.size(); i++) {
        if(ethernets[i]->get_portNum() == port_num) {
            if(ethernets[i]->get_empty()) {
                ethernets[i]->set_sockfd(sockfd);
                ethernets[i]->set_empty(false);
                ethernets[i]->set_IP_connect_to(ip_addr);
                well_done = true;
            }
            else
                cout << "sorry! " << port_num << " is already connected" << endl << endl;
        }
    }
    
    string result;
    if (well_done) {
        result = "<connectDone>" + listen_port + "</connectDone>";
        send(sockfd, result.c_str(), strlen(result.c_str()), 0);
        
        update_routing_table(ip_addr, port_num, "1", sockfd, false);
    }
    else {
        result = "<connectError>";
        send(sockfd, result.c_str(), strlen(result.c_str()), 0);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////

////// age router bud ke dc mishod:
// bayad tamame host haye un routere + ehte routeraro az jadvalesh hazf kone o befreste be baghie
void Router::disconnect_request(int sockfd, fd_set readfds) { //darkhaste dc az host ya router !!
	bool found = false;
    bool is_host = true;
    string dest = "";
    string ethNum = "";
    
	for(int i = 0; i < ethernets.size(); i++) {
        if(ethernets[i]->get_sockfd() == sockfd) {
            found = true;
            dest = ethernets[i]->get_IP_connect_to();
            ethNum = ethernets[i]->get_portNum();
            is_host = ethernets[i]->get_is_host();
            
            ethernets[i]->set_router_port("notSet");
            ethernets[i]->set_empty(true);
            ethernets[i]->set_is_host(true);
            ethernets[i]->set_sockfd(-1);
            ethernets[i]->set_IP_connect_to("notSet");
        } 
	}
    
	if(!found)
		cout << "Sorry! But " << sockfd << " doesn't exist !! :P" << endl << endl;
    else {
        FD_CLR(sockfd, &readfds);
        cout << "Successfully Disconnected!" << endl << endl;
        print_ethernets();
        
        if (is_host) {
            deleteFrom_routing_table(dest, sockfd);
        } else {
            deleteRouter_routing_table(ethNum, sockfd);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////

////// age router bud ke dc mishod:
// bayad tamame host haye un routere + ehte routeraro az jadvalesh hazf kone o befreste be baghie
void Router::disconnect_eth(string ethNum, fd_set readfds) //#Disconnect eth0
{
	bool found;
	int sockfd;
    bool is_host = true;
    string dest = "";
    
	for(int i = 0; i < ethernets.size(); i++) {
        if(ethernets[i]->get_portNum() == ethNum) {
            found = true;
            dest = ethernets[i]->get_IP_connect_to();
            is_host = ethernets[i]->get_is_host();
            sockfd = ethernets[i]->get_sockfd();

            ethernets[i]->set_router_port("notSet");
            ethernets[i]->set_empty(true);
            ethernets[i]->set_sockfd(-1);
            ethernets[i]->set_IP_connect_to("notSet");
            
            if(!ethernets[i]->get_is_host()) {
                string toSend = "<DC>";
            
                if(sockfd != -1)
                    send(sockfd, toSend.c_str(), strlen(toSend.c_str()), 0);
            }
            
            ethernets[i]->set_is_host(true);
        }
	}
    
	if(!found)
		cout << "Sorry! But " << ethNum << " doesn't exist !!" << endl << endl;
    else {
        FD_CLR(sockfd, &readfds);
        cout << "Successfully Disconnected!" << endl << endl;
        print_ethernets();
        
        if (is_host) {
            deleteFrom_routing_table(dest, sockfd);
        } else {
            deleteRouter_routing_table(ethNum, sockfd);
        }
    }
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::print_ethernets() {
	cout << endl << "############### .. Current Ethernets .. ###############" << endl;
    
	for(int i = 0; i < ethernets.size(); i++) {
		cout << ethernets[i]->get_portNum() << " -- ";
		cout << "IP: " << ethernets[i]->get_IP() << " -- ";
		cout << "Cost: " << ethernets[i]->get_cost() << " -- ";
		cout << "IsEmpty: " << (ethernets[i]->get_empty()?"Yes":" No") << " -- ";
		cout << "IsHost: " << (ethernets[i]->get_is_host()?"Yes":"No") << endl;
	}
    cout << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::update_routing_table(string dest, string eth, string cost, int sockfd, bool cost_changed) {
    bool is_exist = false;
    DistVectRow* updated = new DistVectRow("notSet", "notSet");
    
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_destination() == dest) {
            is_exist = true;
            
            if (cost_changed == true) {
                routing_table[i]->set_next_hop(eth);
                routing_table[i]->set_cost(cost);
                updated = routing_table[i];
            } else {
                if (routing_table[i]->get_cost() > cost) {
                    string eth_sfd = find_eth_of_sockfd(sockfd);
                    routing_table[i]->set_next_hop(eth_sfd);
                    routing_table[i]->set_cost(cost);
                    updated = routing_table[i];
                }
            }
            
        }
    }
    
    if (!is_exist) {
        DistVectRow* row = new DistVectRow(dest, eth);
        
        string eth_sfd = find_eth_of_sockfd(sockfd);
        row->set_next_hop(eth_sfd);
        row->set_cost(cost);
        
        routing_table.push_back(row);
        updated = row;
    }
    
    
    //broadcast be hameye routera be joz sender !
    if (updated->get_destination() != "notSet") {
        string costChanged = "0";
        if (cost_changed == true) costChanged = "1";
        string to_send = "<broadcastRT>" + updated->get_destination() + "</dest>" + updated->get_next_hop() + "</nextHop>" + updated->get_cost() + "</cost>" + costChanged + "</costChanged>";
        
        for (int i = 0; i < ethernets.size(); i++) {
            if ((!ethernets[i]->get_is_host())/* && (ethernets[i]->get_sockfd() != sockfd)*/) {
                int sfd = ethernets[i]->get_sockfd();
                if (sfd != sockfd) //ke hey beham nafrestan !
                    send(sfd, to_send.c_str(), strlen(to_send.c_str()), 0);
            }
        }
    }
    
    print_routing_table();
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::deleteFrom_routing_table(string dest, int sockfd) {
    //cout << "inja tooye delete host, dest: " << dest << endl;
    
    bool is_exist = false;
    
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_destination() == dest) {
            is_exist = true;
            
            routing_table.erase(routing_table.begin() + i);
        }
    }
    
    //broadcast be hameye routera be joz sender !
    if (is_exist) {
        string to_send = "<deleteFromRT>" + dest + "</dest>";
        
        for (int i = 0; i < ethernets.size(); i++) {
            if ((!ethernets[i]->get_is_host())/* && (ethernets[i]->get_sockfd() != sockfd)*/) {
                int sfd = ethernets[i]->get_sockfd();
                if (sfd != sockfd) { //ke hey beham nafrestan !
                    send(sfd, to_send.c_str(), strlen(to_send.c_str()), 0);
                }
            }
        }
    }
    
    
    print_routing_table();
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::deleteRouter_routing_table(string eth, int sockfd) {
    //cout << "inja tooye delete router, eth: " << eth << endl;
    
    vector<string> deleted;
    
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_next_hop() == eth) {
            string dest = routing_table[i]->get_destination();
            deleted.push_back(dest);
        }
    }
    
    for (int i = 0; i < deleted.size(); i++) {
        deleteFrom_routing_table(deleted[i], sockfd);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router::print_routing_table() {
    cout << endl << "#################### .. Routing Table .. ####################" << endl;
    for (int i = 0; i < routing_table.size(); i++) {
        cout << "Dest: " + routing_table[i]->get_destination() + " --- Next Hop: " + routing_table[i]->get_next_hop() + " --- Cost: " + routing_table[i]->get_cost() << endl;
    }
    cout << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string Router::find_eth_of_sockfd(int sockfd) {
    for (int i = 0; i < ethernets.size(); i++) {
        if (ethernets[i]->get_sockfd() == sockfd) {
            return ethernets[i]->get_portNum();
        }
    }
    
    return "notFound";
}


/////////////////////////////////////////////////////////////////////////////////////////////////



int Router::find_sockfd_of_eth(string eth) {
    for (int i = 0; i < ethernets.size(); i++) {
        if (ethernets[i]->get_portNum() == eth) {
            return ethernets[i]->get_sockfd();
        }
    }
    
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


bool Router::eth_is4host(string eth) {
    for (int i = 0; i < ethernets.size(); i++) {
        if (ethernets[i]->get_portNum() == eth) {
            if (ethernets[i]->get_is_host()) {
                return true;
            }
        }
    }
    
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


//routere aval !!
void Router::ping(int sockfd, string ping_ip, string ip_addr) {
    
    bool in_table = false;
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_destination() == ping_ip) {
            in_table = true;
            string e = routing_table[i]->get_next_hop();
            int sfd = find_sockfd_of_eth(e);
            
            string eth = routing_table[i]->get_next_hop();
            if (eth_is4host(eth)) { //age hoste khodesh bud ke zudi be darkhastkonandeye ping begu !!
                string result = "<pingRes>";
                for (int i = 0; i < 5; i++) {
                    stringstream ss;
                    ss << (i + 1);
                    string str = ss.str();
                    
                    result += "8 bytes from " + ping_ip + ": icmp_seq=" + str + " ttl=64\n";
                }
                
                result += "</pingRes>";
                send(sockfd, result.c_str(), strlen(result.c_str()), 0);
                
                string pinged = "<pingRes>" + ip_addr + " Pinged You!</pingRes>";
                send(sfd, pinged.c_str(), strlen(pinged.c_str()), 0);
            }
            else { //age bayad next_hopo midid !
                string ping_msg = "<response2ping>" + ip_addr + "</ip1>" + ping_ip + "</pingIP>";
                
                send(sfd, ping_msg.c_str(), strlen(ping_msg.c_str()), 0);
            }
        }
    }
    
    
    if (!in_table) {
        string to_send = "<pingRes>";
        for (int i = 0; i < 5; i++) {
            stringstream ss;
            ss << (i + 1);
            string str = ss.str();
            
            to_send += "From " + ping_ip + " icmp_seq=" + str + " Destination Host Unreachable\n";
        }
        to_send += "</pingRes>";
        send(sockfd, to_send.c_str(), strlen(to_send.c_str()), 0);
    }
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////

//mogheye raft !!
void Router::response2ping(int sockfd, string ip1, string ping_ip) {
    
    bool in_table = false;
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_destination() == ping_ip) {
            in_table = true;
            string e = routing_table[i]->get_next_hop();
            int sfd = find_sockfd_of_eth(e);
            
            string eth = routing_table[i]->get_next_hop();
            if (eth_is4host(eth)) { //age hoste khodesh bud be ghabli begu male mane, jahate ersal avaz she !!
                string result = "<foundPing>" + ip1 + "</ip1>" + ping_ip + "</pingIP>";
                
                send(sockfd, result.c_str(), strlen(result.c_str()), 0);
                
                string pinged = "<pingRes>" + ip1 + " Pinged You!</pingRes>";
                send(sfd, pinged.c_str(), strlen(pinged.c_str()), 0);
            }
            
            else { //age bayad next_hopo midid !
                string ping_msg = "<response2ping>" + ip1 + "</ip1>" + ping_ip + "</pingIP>";
                
                send(sfd, ping_msg.c_str(), strlen(ping_msg.c_str()), 0);
            }
        }
    }
 
}



/////////////////////////////////////////////////////////////////////////////////////////////////

//mogheye bargasht !!
void Router::found_ping(int sockfd, string ip1, string ping_ip) {
    
    bool in_table = false;
    for (int i = 0; i < routing_table.size(); i++) {
        if (routing_table[i]->get_destination() == ip1) {
            in_table = true;
            string e = routing_table[i]->get_next_hop();
            int sfd = find_sockfd_of_eth(e);
            
            string eth = routing_table[i]->get_next_hop();
            if (eth_is4host(eth)) { //age hoste khodesh bud besh befrest 5taro
                string result = "<pingRes>";
                for (int i = 0; i < 5; i++) {
                    stringstream ss;
                    ss << (i + 1);
                    string str = ss.str();
                    
                    result += "8 bytes from " + ping_ip + ": icmp_seq=" + str + " ttl=64\n";
                }
                
                result += "</pingRes>";
                
                send(sfd, result.c_str(), strlen(result.c_str()), 0);
            }
            
            else { //vagarna be ba'di (be samte ip1) begu ping_ip ro yaftam
                string result = "<foundPing>" + ip1 + "</ip1>" + ping_ip + "</pingIP>";
                
                send(sfd, result.c_str(), strlen(result.c_str()), 0);
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router :: Trace ( string packet )
{
	string ethNum;
	int situation= isConnected(packet,ethNum);
	if(situation==0)	// host o man daram hala packet o barmigardoonam !
		turnBackPacket(packet);
	else if(situation==1)	// host o nadashtam mifrestam router e badi
		sendNextRouter(packet,ethNum);
	else
		cout<<"bikhial agha ! :D nadarim kollan ! "<<endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


bool Router :: ifSame ( string packet )	// fd<trace>0IP:D/ -----> age 2 ta host b ye router vasl bashan !
{
	string alaki=reverse(packet);
	int p=alaki.find('/');
	int q=alaki.find('D');
	if(q==p+1)	return true ;
	else		return false ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router :: turnBackPacket(string packet)	// in tabe oonjai seda mishe k router ip o motassel b khodesh peida mikone , yani ye bar !
{

	bool sameRouter=ifSame(packet);
	if(sameRouter)		// yani host i k trace zade o ip i k tracesh karde too ye router!
	{
		int sockfd=getSockfdPacket(packet);
        int f=packet.find('>');
        packet[f+1]='1';
        packet=packet+listen_port+"/2";
        send(sockfd, packet.c_str(), strlen(packet.c_str()), 0) ;
	}
	else
	{
		int f=packet.find('>');
		packet[f+1]='1';
		string newPacket=packet+listen_port+'/'+'2';
		
		string port=portFromPacket(newPacket);
		
		int sockfd=getSockfdByPort(port);
		
		string packetSend=plusPacket(newPacket);
		send(sockfd, packetSend.c_str(), strlen(packetSend.c_str()), 0) ;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router :: getSockfdByPort(string port)
{
	int sockfd=-1;
	for(int i=0;i<ethernets.size();i++)
	{
		if(ethernets[i]->get_router_port()==port)
		{
			sockfd=ethernets[i]->get_sockfd();
		}
	}
	return sockfd;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router :: sendNextRouter(string packet,string ethNum)   // har router ip khodesho (inja port i k roosh listen mikone chon unique)
{							     // be akhare packet ezafe mikone o mifreste be router e badi !
	string newPacket=packet+listen_port+"/";
	//cout<<"new packet to send next router --->"<<newPacket<<"*"<<endl;
	int sockfd=getSockfd(ethNum);
	
	send(sockfd, newPacket.c_str(), strlen(newPacket.c_str()), 0) ;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router :: getSockfd(string ethNum)
{
	int sockfd=-1;
	for(int i=0;i<ethernets.size();i++)
	{
		if(ethernets[i]->get_portNum()==ethNum)
		{
			sockfd=ethernets[i]->get_sockfd();
		}
	}
	return sockfd;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string Router :: ip (string packet)	// in ghesmate data e packet k hamoon IP misharo barmigardoone !
{	// <Trace>0IP:D
	int p=packet.find("<Trace>");
	int q=packet.find(":D");
	string ip=packet.substr(p+7+1,q-p-7-1);
	return ip;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router :: isConnected(string packet,string &ethNum)     // age 0 befreste yani host e ping shodaro peida kardam b khodam vasle
{					  // age 1 befreste yani peidash kardam amma b router e badi vasle bayad befrestam b oon (header)
    // age 2 befreste yani kolln chenin chizi peida nakarde too routing tablesh ! (header)
	int situation=2;
    for(int i=0; i<routing_table.size() ;i++)
    {
		if(routing_table[i]->get_destination()==ip(packet))
		{
			ethNum=routing_table[i]->get_next_hop();
			if( host_interface(routing_table[i]->get_next_hop()) )
				situation=0;
			else
				situation=1;
			break;
		}
    }
	//cout<<"connecte to this router or not ! "<<situation<<endl;
	return situation;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


bool Router :: host_interface(string interface)		// yani in interface b chi vasle , be host ya router !
{							// true --------------->  host
	for(int i=0;i<ethernets.size();i++)
	{
		if(ethernets[i]->get_portNum()==interface)
		{
			if(ethernets[i]->get_is_host())
				return true ;
			else
				return false ;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string intToString(int num)
{
	if(num==0)	return "0";
	if(num==1)	return "1";
	if(num==2) 	return "2";
	if(num==3)	return "3";
	if(num==4)	return "4";
	if(num==5) 	return "5";
	if(num==6)	return "6";
	if(num==7) 	return "7";
	if(num==8)	return "8";
	if(num==9) 	return "9";
	if(num==10)	return "10";
    
}



/////////////////////////////////////////////////////////////////////////////////////////////////


string Router :: makeTracePacket(string msg,int fd)	// #Trace 10.10.10.4
{
	msg+="<!!!>";
	//cout<<fd<<" - make trace "<<msg<<endl;
	string fdStr=intToString(fd);
	string out=fdStr+"<Trace>0";
    
	int p=msg.find("#Trace");
	int q=msg.find("<!!!>");
	string ip=msg.substr(p + 6+1, q - p - 7);
	out=out+ip+":D/";
    
	return out;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


void Router :: backingTrace(string packet)
{
	bool lastRouter=checkLastRouter(packet);
	//cout<<"backing "<<packet<<endl;
	if(!lastRouter)
	{
		string port=portFromPacket(packet);
		int sockfd=getSockfdByPort(port);
		string PacketSend=plusPacket(packet);
		
		send(sockfd, PacketSend.c_str(), strlen( PacketSend.c_str()), 0) ;
	}
	else // b host bege natije trace o !
	{
		int sockfd=getSockfdPacket(packet);
		send(sockfd, packet.c_str(), strlen( packet.c_str()), 0) ;	// trace inja tamoom mishe !
		
	}
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////


int Router :: getSockfdPacket(string packet)	// sockfd avale packet ezafe shode !
{
	int a=packet.find('<');
	string sockfdStr=packet.substr(0,a);
	int sockfd=atoi(sockfdStr.c_str());
	return sockfd;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string Router :: reverse(string newPacket)
{
	if (newPacket.length()==1) return newPacket;
	string alaki=newPacket;
	for(int i=0;i<newPacket.length();i++)
	{
		alaki[i]=newPacket[newPacket.length()-1-i];
	}
	return alaki;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string Router :: portFromPacket(string newPacket)	// bege alan be kodoom port (router) bayad befrestam !
{
	int f=newPacket.find_last_of('/');
	string numStr=newPacket.substr(f+1);
	int num=atoi(numStr.c_str());
	string alaki=reverse(newPacket);
    
	int p;
	for(int i=1;i<=num;i++)
	{
		p=alaki.find('/');
		alaki[p]='*';
	}
	int q=alaki.find('/');
	
	string portAlaki=alaki.substr(p+1,q-p-1);
	string port=reverse(portAlaki);
	
	return port;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


string Router :: plusPacket(string newPacket)		// be shomare router ezafe kone !
{
	string alaki=reverse(newPacket);
	int p=alaki.find('/');
	string numStr=alaki.substr(p+1);
	string numStr2=reverse(numStr); // packet - {/num}
    
	numStr=alaki.substr(0,p);
	int num=atoi(numStr.c_str());
	num++;
	string n=intToString(num);
    
	string out =numStr2+'/'+n;
	return out;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


bool Router :: checkLastRouter(string packet)	// mige b akharin router residam , routeri k trace az host e oon oomade!
{						// true ---> last router , false ---> first router
	
	int f=packet.find_last_of('/');
	string numStr=packet.substr(f+1);
	int num=atoi(numStr.c_str());
    
	string alaki=reverse(packet);
	int p;
	for(int i=1;i<num;i++)
	{
		p=alaki.find('/');
		alaki[p]='*';
	}
	p=alaki.find('/');
	int q=alaki.find('D');
    
	if(q==p+1)	return true ;
	else		return false ;
    
}




















