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
    
    int64_t ooo = sizeof(YorkNet::YorkNetwork::MessageHeader);
    char aa[] = "wwwsss";
    
	YorkSocketServer myserver;
	myserver.StartServer();
    
    
//    char *bb  = myserver.createBuffer(aa, 1,1,1);

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
