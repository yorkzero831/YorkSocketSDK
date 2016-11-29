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
        void connectTo(std::string ip, int port = DEFULT_PORT);
        
        
        virtual void didGetMessage(const char *inMessage,const Header &header){};
        virtual void didGetFile(const char *inMessage,const Header &header){};
        
        
    private:
        std::string ipAddr;
        int         portNo;
        int         sockID;
        
        std::thread waitMesThread;
        std::thread cmdSysThread;
        
        void readFromServer();
        void commandSystem();
        
        void writeToServer(std::string message, int64_t tag, int64_t IOB = 1, int64_t TOB =1);
        void sentFileToServer(std::string fileName, std::string fileType);
        
        //void (* didGetFileDelegate)(const char *inMessage,const Header &header);
        
	};

} /* namespace York */

#endif /* YORKSOCKETCLIENT_H_ */
