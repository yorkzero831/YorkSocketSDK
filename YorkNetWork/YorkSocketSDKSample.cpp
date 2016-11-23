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

	YorkSocketServer myserver;
	myserver.StartServer(10832);

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
