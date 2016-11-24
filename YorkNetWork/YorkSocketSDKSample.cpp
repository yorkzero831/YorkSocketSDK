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
    
    char a[] = "00000005";
    int aa = YorkNetwork::charToInt(a);
	YorkSocketServer myserver;
	myserver.StartServer();

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
