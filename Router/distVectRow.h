#ifndef __DISTVECTROW_H__
#define __DISTVECTROW_H__

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
#include <sys/time.h>
#include "ethernet.h"


using namespace std;


class DistVectRow {
public:
    DistVectRow(string d, string e) { destination = d; next_hop = e; cost = "1"; }
    
    void set_destination(string d) { destination = d; }
    void set_next_hop(string e) { next_hop = e; }
    void set_cost(string c) { cost = c; }
    
    string get_destination() { return destination; }
    string get_next_hop() { return next_hop; }
    string get_cost() { return cost; }
    
private:
    string destination;
    string next_hop;
    string cost;
};


#endif

