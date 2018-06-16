#include "host.h"


int main(int argc, char *argv[]) {
	struct sockaddr_in serv_addr;
	struct hostent *server;
    string _ip = "127.0.0.1"; //
    char* IP = (char*)_ip.c_str(); //argv[1];
    int PORT;
	char buffer[MESSAGE_SIZE];
    struct timeval timeout;

    bool dc = false;
    
    Host* h = new Host();
    
DC:
    string input, result = "";
    bool set_port = false;
    while (!set_port) {
        cin >> input;
        if (input == "#IP") {
            cin >> input;
            h->set_ip_addr(input);
            
            cout << "IP Address Set!" << endl << endl;
        }
        
        else if (input == "#Gateway") {	//#Gateway 2021 eth0
            if (h->get_ip_addr() != "notSet") {
                cin >> input;
                PORT = atoi(input.c_str());
            
                cin >> input ;
                h->set_eth(input);
            
                result += "<Gateway>" + h->get_eth() + "</eth>" + h->get_ip_addr() + "</ip>";
		
                set_port = true;
            } else {
                cout << "You should first set IP Address!" << endl << endl;
            }
        }
        
        else {
            cout << "You Should Connect First!" << endl << endl;
        }
    }
    
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) cerr << "Error Opening Socket!" << endl;
	server = gethostbyname(IP);
	if (server == NULL) {
		cerr << "no such host" << endl;
		exit(0);
	}
    
    stringstream ss;
    ss << sock_fd;
    string sfd = ss.str();
    h->set_sockfd(sfd);

	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP);
	serv_addr.sin_port = htons(PORT);
	int temp;
        
	if ((temp=connect(sock_fd, (sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        cerr << "Connecting Error!" << endl;
        dc = true;
        return 0;
    }
        
    
    bzero(buffer, MESSAGE_SIZE);
    strcpy(buffer, result.c_str());
        
    //write connect!!
    int n = write(sock_fd, buffer, strlen(buffer));
    if (n < 0) cerr << "Error: Writing to the Socket" << endl;
    
    
    
    ///////////////// select:
    
    fd_set master_fd, read_fd;
	FD_ZERO(&master_fd);
	FD_SET(sock_fd, &master_fd);
	FD_SET(0, &master_fd);
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    
    while (!dc) {
        int p, q;
        
        FD_ZERO(&read_fd);
		FD_SET(sock_fd, &read_fd);
		FD_SET(0, &read_fd);
        
		if (select(sock_fd + 1, &read_fd, NULL, NULL, NULL) == -1) {
			cerr << "Select() Error!" << endl;
			exit(1);
		}
        
   
        if (FD_ISSET(0, &read_fd)) {
            cin >> input;
            result = "";
            
            if (input == "#IP") {
                cin >> input;
                h->set_ip_addr(input);
                cout << "IP Address Set!" << endl << endl;
            }
            
            else if (input == "#Disconnect") {
                dc = true;
                string res = "<DC>";
                send(sock_fd, res.c_str(), strlen(res.c_str()), 0);
                goto DC;
            }
            
            else if (input == "#Ping") {
                cin >> input;
                
                result += "<ping>" + input + "</ping>" + h->get_ip_addr() + "</ip>";
                    
                bzero(buffer, MESSAGE_SIZE);
                strcpy(buffer, result.c_str());
                
                //write
                int n = write(sock_fd, buffer, strlen(buffer));
                if (n < 0) cerr << "Error: Writing to the Socket" << endl;
            }
            
            else if (input == "#Trace") {
                cin >> input ;
                result = "#Trace " + input;
                
                int n = write(sock_fd,result.c_str(), strlen(result.c_str()));
                if (n < 0) cerr << "Error: Writing to the Socket" << endl;
            }
        }
        
        
        if (FD_ISSET(sock_fd, &read_fd)) {
            //read
            bzero(buffer, MESSAGE_SIZE);
            int n1 = read(sock_fd, buffer, MESSAGE_SIZE);
            if (n1 < 0) cerr << "Error: Reading from the Socket" << endl;
            
            string output = (string)buffer;
            
            if (p = output.find("<connectDone>") != string::npos) { //Connect Done
                q = output.find("</connectDone>");
                string lp = output.substr(p + 12, q - p - 12);
                h->connect_router(lp);
                cout << "Done!" << endl << endl;
                
            } else if (output == "<connectError>") { //Connect Fail
                cout << "Connecting Error! There is no Ethernet for You!" << endl << endl;
                goto DC;
                
            } else if (p = output.find("<pingRes>") != string::npos) { //Ping Result
                q = output.find("</pingRes>");
                string ping_res = output.substr(p + 8, q - p - 8);
                
                cout << ping_res << endl;
                
            } else if (p = output.find("<Trace>1") != string::npos) { //Trace
                showTrace(output);
                
            } else {
                cout << "###############" << output << "###############" << endl << endl;
            }
        }
        
	} // while(dc)
    
	FD_CLR(sock_fd, &read_fd);
    FD_CLR(sock_fd, &master_fd);
    
	return 0;
}


/////////////////////////////////////////////////////////////////////////////


string reverse(string newPacket)
{
	if (newPacket.length()==1) return newPacket;
	string alaki=newPacket;
	for(int i=0;i<newPacket.length();i++)
	{
		alaki[i]=newPacket[newPacket.length()-1-i];
	}
    
	return alaki;
}


/////////////////////////////////////////////////////////////////////////////


void showTrace(string packet)
{
	string alaki=reverse(packet);
	int p=alaki.find('/');
	string noNumber=alaki.substr(p+1);
    
	string numStr=alaki.substr(0,p);
	int num=atoi(numStr.c_str());	// tedade router haye masir + 1
	cout<<"Number of Routers: "<<num<<endl;
	
	string port;
	
	int begin=0;
	int end;
	vector<string> routers;
	for(int i=1;i<num;i++)
	{
		end=noNumber.find('/');
		port=noNumber.substr(begin,end-begin);
		noNumber[end]='*';
		begin=end+1;
		port=reverse(port);
		routers.push_back(port);
	}
	
	for(int i=0;i<routers.size();i++)
	{
		cout<<routers[routers.size()-i-1]<<endl;
	}
	
}


