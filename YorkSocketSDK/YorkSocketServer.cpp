/*
 * YorkSocketServer.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */
#include "YorkSocketServer.h"

namespace YorkNet {
	
	YorkSocketServer::YorkSocketServer() {

		// TODO Auto-generated constructor stub
	
	}
	YorkSocketServer::~YorkSocketServer() {
		close(listenSocket);
	}
	

	void YorkSocketServer::StartServer(int portNum)
	{
		clients = map<string, int>();
		if ((listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			ErrorMessage error = ErrorMessage(CANNOTCREATESOCKT, "server error");
			ShowErrorMessage(error);
			return;
		}
		else
		{
			cout << "Success on create socket" << endl;
		}

		//clen = sizeof(client_addr);
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(portNum);
		bzero(&(server_addr.sin_zero), 8);

		if (bind(listenSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		{
			ErrorMessage error = ErrorMessage(CANNOTBLINDSERVER,"bind error");
			ShowErrorMessage(error);
			return;
		}
		else
		{
			cout << "Success on binding server" << endl;
		}

		if (listen(listenSocket, 5) < 0)
		{
			ErrorMessage error = ErrorMessage(CANNOTLISITENPORT, "Listen error");
			ShowErrorMessage(error);
			return;
		}
		else
		{
			cout << "Success on listen port:" << portNum << endl;
		}

		cout << "Create thread on listening client message"<<endl;

		connectThread = thread(&YorkNet::YorkSocketServer::runServer, this);
		connectThread.detach();

		waitMesThread = thread(&YorkNet::YorkSocketServer::waitingMessage, this);
		waitMesThread.detach();
		//runServer();
		string input = "";
		while (true)
		{
			cin >> input;
			if (input == "clientCount")
			{
				cout << clients.size() << endl;
			}
			else if (input == "listClient")
			{
				map<string, int>::iterator it;
				for (it = clients.begin(); it != clients.end(); it++)
				{
					cout << "ADDR:" << it->first << " ID:" << it->second << endl;
				}
			}
			else if (input == "sentMessage")
			{
				cout << "Input ClientID" << endl;
				int id = 0;
				cin >> id;
				cout << "Input Message" << endl;
				string mess = "";
				cin >> mess;

				SentMessageTo(id, mess);
			}
			else if (input == "sentMessageAll")
			{
				cout << "Input Message" << endl;
				string mess = "";
				cin >> mess;

				sentMessageToALL(mess);
			}
			else
			{
				cout << "WrongParamater" << endl;
			}
		}
		
	}



	void YorkSocketServer::runServer ()
	{
		
		while (1)
		{
			sin_size = sizeof(struct sockaddr_in);
			if ((clientSocket = accept(listenSocket, (struct sockaddr*)&remote_addr, &sin_size)) < 0)
			{
				continue;
			}
			else
			{
				ostringstream myN;
				myN << inet_ntoa(remote_addr.sin_addr) << ":" << remote_addr.sin_port;

				string ctrr(myN.str());
				cout << "received a connection from " << ctrr << endl;
				clients.insert(pair<string, int>(ctrr, clientSocket));
				if (!fork())
				{
					SentMessageTo(clientSocket, "You are In!!/n");
					exit(0);
				}
			}
			

		}
	}

	void YorkSocketServer::waitingMessage()
	{
		while (1)
		{
			if (clients.size() == 0)
				continue;
			map<string, int>::iterator it;
			for (it = clients.begin(); it != clients.end(); it++)
			{
				
				/*thread waitForClient = thread(&YorkNet::YorkSocketServer::waitingMessagePerClient,this,it);
				waitForClient.detach();*/
				int thisSocket = it->second;
				size_t buf_Pointer = 0;
				char thisBuf[MAXBUFF] = { 0 };
				while (buf_Pointer < MAXBUFF)
				{
					fcntl(thisSocket, F_SETFL,  O_NONBLOCK);
					if (read(thisSocket, &thisBuf[buf_Pointer], 1) <= 0)
					{
						if (errno == EWOULDBLOCK)
						{
							//cout << "nothing" << endl;
							break;;
						}
						else
						{
							cout << it->first << "removed" << endl;
							clients.erase(it);
							int count = clients.size();
							break;
						}
					}


					if (buf_Pointer > 0 && '\n' == thisBuf[buf_Pointer])
					{
						cout << it->first << " Received:" << thisBuf << endl;
						DidRecivedMessage(it->second, it->first);
						break;
					}

					buf_Pointer++;
				}

			}

		}
		
	}




	void YorkSocketServer::StopServer()
	{
		close(listenSocket);
	}

	void YorkSocketServer::DidRecivedMessage(const int& clientID, const string& Addr)
	{

	}

	void YorkSocketServer::SentMessageTo(int socketID, string words)
	{
		int sentSize = words.size();
		char sentChar[sentSize];
		strcpy(sentChar, words.c_str());
		if (send(socketID, sentChar, sentSize, 0) == -1) {
			perror("send error！");
		}
		//close(socketID);
	}

	void YorkSocketServer::sentMessageToALL(string words)
	{
		int sentSize = words.size();
		char sentChar[sentSize];
		strcpy(sentChar, words.c_str());

		map<string, int>::iterator it;
		for (it = clients.begin(); it != clients.end(); it++)
		{
			fcntl(it->second, F_SETFL, O_NONBLOCK);
			if (send(it->second, sentChar, sentSize, 0) == -1)
			{
				if (errno == EWOULDBLOCK)
				{
					//cout << "nothing" << endl;
					continue;
				}
				perror("send error！");
			}
		}
	}

} /* namespace York */
