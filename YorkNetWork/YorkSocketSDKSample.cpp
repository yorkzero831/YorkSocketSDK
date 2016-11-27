//============================================================================
// Name        : YorkSocketSDK.cpp
// Author      : York Yu
// Version     :
// Copyright   : A SDK to make socket easier
// Description : in C++, Ansi-style
//============================================================================

#include <iostream>
#include "YorkSocketServer.h"
using namespace std;
using namespace YorkNet;


int main() {
    
    int64_t ooo = sizeof(YorkNet::YorkNetwork::Header);
    char aa[] = "wwwsss";
    char *bb  = YorkNetwork::createBuffer(aa, 1,1,1);
    
    char cc[38];
    
    LOOP(38)
    {
        cc[ii] = bb[ii];
        std::cout<<bb[ii];
    }
    
	YorkSocketServer myserver;
	myserver.StartServer();

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
