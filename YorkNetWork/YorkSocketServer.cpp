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
	

	void YorkSocketServer::StartServer(int portNum )
	{
		clients = std::map<std::string, int>();
		if ((listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			ErrorMessage error = ErrorMessage(CANNOTCREATESOCKT, "server error");
			ShowErrorMessage(error);
			return;
		}
		else
		{
			std::cout << "Success on create socket" << std::endl;
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
			std::cout << "Success on binding server" << std::endl;
		}

		if (listen(listenSocket, 5) < 0)
		{
			ErrorMessage error = ErrorMessage(CANNOTLISITENPORT, "Listen error");
			ShowErrorMessage(error);
			return;
		}
		else
		{
			std::cout << "Success on listen port:" << portNum << std::endl;
		}

		std::cout << "Create thread on listening client message"<<std::endl;
        
        //std::ifstream ins("./file/3.json",std::ifstream::in);

		connectThread = std::thread(&YorkNet::YorkSocketServer::runServer, this);
		connectThread.detach();

		waitMesThread = std::thread(&YorkNet::YorkSocketServer::waitingMessage, this);
		waitMesThread.detach();
        
        cmdSysThread = std::thread(&YorkNet::YorkSocketServer::commandSystem, this);
        cmdSysThread.detach();
		//runServer();
		
        while (true) {
            std::this_thread::sleep_for(hearBeatC);
        }
		
	}
    
    void YorkSocketServer::commandSystem()
    {
        //while (true)
        //{
        std::this_thread::sleep_for(hearBeatC);
        
            std::string input = "";
            std::cin >> input;
            if (input == "clientCount")
            {
                std::cout << clients.size() << std::endl;
            }
            else if (input == "listClient")
            {
                std::map<std::string, int>::iterator it;
                for (it = clients.begin(); it != clients.end(); it++)
                {
                    std::cout << "ADDR:" << it->first << " ID:" << it->second << std::endl;
                }
            }
            else if (input == "sentMessage")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                std::cout << "Input Message" << std::endl;
                std::string mess = "";
                std::cin >> mess;
                
                SentMessageTo(id, mess ,1);
            }
            else if (input == "sentMessageAll")
            {
                std::cout << "Input Message" << std::endl;
                std::string mess = "";
                std::cin >> mess;
                
                SentMessageToALL(mess ,1);
            }
            else if(input == "sentFile")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                std::string filePath = YorkNetwork::getDirPath("1.json");
                
                SentFileTo(id, filePath);
            }
            
        commandSystem();
        //}
    }



	void YorkSocketServer::runServer ()
	{
		
		while (1)
		{
            std::this_thread::sleep_for(hearBeatC);
            
			sin_size = sizeof(struct sockaddr_in);
			if ((clientSocket = accept(listenSocket, (struct sockaddr*)&remote_addr, &sin_size)) < 0)
			{
				continue;
			}
			else
			{
				std::stringstream myN;
				myN << inet_ntoa(remote_addr.sin_addr) << ":" << remote_addr.sin_port;

				std::string ctrr(myN.str());
				std::cout << "received a connection from " << ctrr << std::endl;
                clients.insert(std::pair<std::string, int>(ctrr, clientSocket));
				//if (!fork())
				{
					SentMessageTo(clientSocket, "You are In!!",1);
					//exit(0);
				}
			}
			
		}
	}

	void YorkSocketServer::waitingMessage()
	{
		while (1)
		{
            //std::ifstream ins("./file/3.json",std::ifstream::in);
            
            std::this_thread::sleep_for(hearBeatC);
			if (clients.size() == 0)
				continue;
            std::map<std::string, int>::iterator it;
			for (it = clients.begin(); it != clients.end(); it++)
			{
				
				/*thread waitForClient = thread(&YorkNet::YorkSocketServer::waitingMessagePerClient,this,it);
				waitForClient.detach();*/
                int timeCount = 0;
				int thisSocket = it->second;
				size_t buf_Pointer = 0;
				char thisBuf[MAX_BUFFER_SIZE] = { 0 };
				while (buf_Pointer < MAX_BUFFER_SIZE)
				{
					fcntl(thisSocket, F_SETFL,  O_NONBLOCK);
					if (read(thisSocket, &thisBuf[buf_Pointer], 1) <= 0)
					{
						if (errno == EWOULDBLOCK)
						{
							//std::cout << "nothing" << std::endl;
							break;;
						}
						else
						{
							std::cout << it->first << " Removed" << std::endl;
							clients.erase(it);
                            close(it->second);
							int count = clients.size();
							break;
						}
					}


					if (buf_Pointer > 0 && endOfStream == thisBuf[buf_Pointer])
					{
						std::cout << it->first << " Received:" << thisBuf << std::endl;
						DidRecivedMessage(it->second, it->first);
						break;
					}

					buf_Pointer++;
				}
                timeCount ++;
                if(timeCount > clients.size())
                    break;

			}
		}
        
	}




	void YorkSocketServer::StopServer()
	{
		close(listenSocket);
	}

	void YorkSocketServer::DidRecivedMessage(const int& clientID, const std::string& Addr)
	{

	}

	void YorkSocketServer::SentMessageTo(int socketID, std::string words, int64_t tag, int64_t IOB, int64_t TOB)
	{
//		unsigned long sentSize = words.size();
//		char sentChar[sentSize + 1];
//        sentChar[sentSize] = endOfStream;
        char *sentChar = YorkNetwork::createBuffer(words, tag, TOB, IOB);
        int64_t size = words.length();
        
		//strcpy(sentChar, words.c_str());
		if (send(socketID, sentChar, size + HEADER_LENGTH, 0) == -1) {
			perror("Send error！");
            std::map<std::string, int>::iterator it;
            for (it = clients.begin(); it != clients.end(); it++)
            {
                if(it->second == socketID)
                {
                    clients.erase(it);
                    return;
                }
            }
		}
		//close(socketID);
	}

	void YorkSocketServer::SentMessageToALL(std::string words, int64_t tag, int64_t IOB, int64_t TOB)
	{
        std::vector<std::string> needToDelete =  std::vector<std::string>();
        
        char *sentChar = YorkNetwork::createBuffer(words, tag, TOB, IOB);
        int64_t size = words.length();

        std::map<std::string, int>::iterator it;
		for (it = clients.begin(); it != clients.end(); it++)
		{
			fcntl(it->second, F_SETFL, O_NONBLOCK);
			if (send(it->second, sentChar, size + HEADER_LENGTH, 0) == -1)
			{
				if (errno == EWOULDBLOCK)
				{
					//std::cout << "nothing" << std::endl;
					continue;
				}
				perror("Send error！");
                needToDelete.push_back(it->first);
			}
		}
        
        for (int i= 0; i<needToDelete.size(); i++) {
            clients.erase(needToDelete.at(i));
        }
	}
    
    void YorkSocketServer::SentFileTo(int socketID, std::string filePath)
    {
        
        
        // count fileBlock sent
        int64_t fileSize = YorkNetwork::getFileSize(filePath);

        char buffer[FILE_BUFFER_SIZE];
        
        size_t fileBlockTotal = fileSize/FILE_BUFFER_SIZE;
        if(fileSize%FILE_BUFFER_SIZE > 0)
            fileBlockTotal++;
        
     
        FILE *fileR = std::fopen("./file/1.json", "r");
        
        if(fileR == NULL)
        {
            fclose(fileR);
            std::cout << "Can not OpenFile "<< filePath << std::endl;
            return;
        }
        
        
        int thisBlockNum = 0;
        
        bzero(buffer, FILE_BUFFER_SIZE);
        int file_block_length = 0;
        
        //char *oo = fgets(buffer, 100, fileR);
        
        while( (file_block_length = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, fileR)) > 0)
        {
            thisBlockNum ++;
            
            std::cout << "file_block_length: " << file_block_length << std::endl;
            std::cout << "content: " << buffer << std::endl;
            
            char *sentChar = YorkNetwork::createBuffer(buffer, 2, fileBlockTotal, thisBlockNum);
            int64_t sentLenth = strlen(buffer);
            
            if(send(socketID, sentChar, sentLenth + HEADER_LENGTH, 0) < 0)
            {
                std::cout << "Error on sending file"  << std::endl;
                break;
            }
            
            bzero(buffer, FILE_BUFFER_SIZE);
            
        }
        //SentMessageTo(socketID, "",1);
        
        fclose(fileR);
        std::cout << "File: "<< filePath<<" Transfer finished"  << std::endl;
        
    }
    
    
    

} /* namespace York */
