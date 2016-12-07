/*
 * YorkSocketServer.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
 */

#ifndef YORKSOCKETSERVER_H_
#define YORKSOCKETSERVER_H_


#define  MAXCLIENTNUM 10

#include"YorkNetwork.h"


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
        std::map<int, std::string> clientSockets = std::map<int, std::string>();

		void bindServer();
		void runServer();
        //void waitingMessage(int socketID, std::string key);
        void ListenToClient(int socketID, std::string key);
        void commandSystem();
        
        
		
	};

} /* namespace York */

#endif /* YORKSOCKETSERVER_H_ */
