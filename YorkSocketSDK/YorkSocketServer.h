/*
 * YorkSocketServer.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKSOCKETSERVER_H_
#define YORKSOCKETSERVER_H_

#define  MAXBUFF 1024
#define  MAXCLIENTNUM 10

#include"YorkNetwork.h"
#include <map>

namespace YorkNet {

	class YorkSocketServer :public YorkNetwork
	{
	public:
		YorkSocketServer();
		~YorkSocketServer();

		void StartServer(int portNum);
		void StopServer();
		void SentMessageTo(int socketID, string words);
		void sentMessageToALL(string words);
		virtual void DidRecivedMessage(const int& clientID,const string& Addr);
		
	private:
		thread connectThread;
		thread waitMesThread;
		unsigned int clen;
		int recvbytes;
		socklen_t sin_size;
		int listenSocket, clientSocket;
		struct sockaddr_in remote_addr;
		struct sockaddr_in server_addr;
		char buf[MAXBUFF];
		pthread_t clientThreadID[MAXCLIENTNUM];
		map<string, int> clients;

		void bindServer();
		void runServer();
		void waitingMessage();
		
	};

} /* namespace York */

#endif /* YORKSOCKETSERVER_H_ */
