/*
 * YorkSocketServer.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKSOCKETSERVER_H_
#define YORKSOCKETSERVER_H_


#define  MAXCLIENTNUM 10

#include"YorkNetwork.h"
#include <map>
#include <vector>

//using namespace::std;

namespace YorkNet {

	class YorkSocketServer :public YorkNetwork
	{
	public:
		YorkSocketServer();
		~YorkSocketServer();

		void StartServer(int portNum = DEFULT_PORT);
		void StopServer();
		void SentMessageTo(int socketID, std::string words, int64_t tag, int64_t IOB = 1, int64_t TOB =1);
		void SentMessageToALL(std::string words, int64_t tag, int64_t IOB = 1, int64_t TOB =1);
        void SentFileTo(int socketID, std::string fileName, std::string fileType);
		//virtual void DidRecivedMessage(const int& clientID,const std::string& Addr);
        
        virtual void didGetMessage(const char *inMessage,const Header &header){};
        virtual void didGetFile(const char *inMessage,const Header &header);
        
		
	private:
		std::thread connectThread;
		std::thread waitMesThread;
        std::thread cmdSysThread;
        
        std::mutex fileMutex;
        
		unsigned int clen;
		int recvbytes;
		socklen_t sin_size;
		int listenSocket, clientSocket;
		struct sockaddr_in remote_addr;
		struct sockaddr_in server_addr;
		char buf[MAX_BUFFER_SIZE];
        
        std::map<int, std::thread*> waitingMessageThreads;
        std::map<std::string, int> clients = std::map<std::string, int>();

		void bindServer();
		void runServer();
        void waitingMessage(int socketID, std::string key);
        void commandSystem();
        
        
		
	};

} /* namespace York */

#endif /* YORKSOCKETSERVER_H_ */
