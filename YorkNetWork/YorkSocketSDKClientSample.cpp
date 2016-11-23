#include <iostream>
#include "YorkSocketClient.h"
using namespace std;
using namespace YorkNet;


int main() {
    
    YorkSocketClient myClient;
    myClient.connectTo("127.0.0.1", 10832);
    
    cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    return 0;
}