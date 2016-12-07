/*
 * YorkSocketClient.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
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
        virtual void connectTo(std::string ip, int port = DEFULT_PORT);
        virtual void commandSystem();
        void disconnect();
        
        int         sockID;

    private:
        std::string ipAddr;
        int         portNo;
        bool        finishedFlag = false;
        
        std::thread waitMesThread;
        std::thread cmdSysThread;
        
        void writeToServer(std::string message, int64_t tag, int64_t IOB = 1, int64_t TOB =1);
        
        virtual void didGetFile(const Header &header);
        
        virtual std::string getDirPath(std::string ins);
	};

} /* namespace York */

#endif /* YORKSOCKETCLIENT_H_ */
