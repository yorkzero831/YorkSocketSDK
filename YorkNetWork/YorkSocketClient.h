/*
 * YorkSocketClient.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKSOCKETCLIENT_H_
#define YORKSOCKETCLIENT_H_

#include"YorkNetwork.h"

namespace YorkNet {

	class YorkSocketClient : public YorkNetwork
	{

	public:
		YorkSocketClient();
		virtual ~YorkSocketClient();
        void connectTo(std::string ip, int port);
        void writeToServer(std::string message);
        
    private:
        std::string ipAddr;
        int         portNo;
        int         sockID;
        
        std::thread waitMesThread;
        std::thread cmdSysThread;
        
        void readFromServer();
        void commandSystem();
        
	};

} /* namespace York */

#endif /* YORKSOCKETCLIENT_H_ */
