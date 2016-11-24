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
		void SentMessageTo(int socketID, std::string words);
		void SentMessageToALL(std::string words);
        void SentFileTo(int socketID, std::string filePath);
		virtual void DidRecivedMessage(const int& clientID,const std::string& Addr);
		
	private:
		std::thread connectThread;
		std::thread waitMesThread;
        std::thread cmdSysThread;
		unsigned int clen;
		int recvbytes;
		socklen_t sin_size;
		int listenSocket, clientSocket;
		struct sockaddr_in remote_addr;
		struct sockaddr_in server_addr;
		char buf[MAX_BUFFER_SIZE];
		pthread_t clientThreadID[MAXCLIENTNUM];
		std::map<std::string, int> clients;

		void bindServer();
		void runServer();
		void waitingMessage();
        void commandSystem();
		
	};

} /* namespace York */

#endif /* YORKSOCKETSERVER_H_ */
