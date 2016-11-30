/*
 * YorkSocketServer.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
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

		//waitMesThread = std::thread(&YorkNet::YorkSocketServer::waitingMessage, this);
		//waitMesThread.detach();
        
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
                
                //sentFileToSocket(id, "1", "json");
                while (1) {
                    std::this_thread::sleep_for(hearBeatC*100);
                    sentFileToSocket(id, "1", "json");
                }
            }
            else if(input == "sentFile1")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                sentFileToSocket(id, "1", "png");
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
                
                std::thread* waitingMessage = new std::thread(&YorkNet::YorkSocketServer::waitingMessage, this, clientSocket, ctrr);
                waitingMessage->detach();
                waitingMessageThreads.insert(std::pair<int, std::thread*>(clientSocket, waitingMessage));
			}
			
		}
	}
    
    void YorkSocketServer::waitingMessage(int socketID, std::string key)
    {
        std::cout<<"Waiting Message For "<<key<<std::endl;
        std::vector<RecivedData>fileContextList = std::vector<RecivedData>();
        std::string thisFileName          = "";
        FileTypes   thisFileType          = FileTypes::NONE;
        while (1)
        {
            std::this_thread::sleep_for(hearBeatC);
            
            bool isChecked_header                       = false;
            bool fileDataBegin                             = false;
            
            Header thisHeader;
            
            
            size_t buf_Pointer                          = 0;
            size_t buf_Context_Pointer                  = 0;
            char headerBuff[HEADER_LENGTH]              = { 0 };
            char *contextBuff                           = nullptr;
            
            if(!isChecked_header)
            {
                while (buf_Pointer < HEADER_LENGTH)
                {
                    //std::this_thread::sleep_for(hearBeatC);
                    fcntl(socketID, F_SETFL,  O_NONBLOCK);
                    if (read(socketID, &headerBuff[buf_Pointer], 1) <= 0)
                    {
                        if (errno == EWOULDBLOCK){ break; }
                        else
                        {
                            std::cout << key << " Removed" << std::endl;
                            clients.erase(key);
                            close(clientSocket);
                            waitingMessageThreads.erase(socketID);
                            return;
                        }
                    }
                    if (buf_Pointer == HEADER_LENGTH-1)
                    {
                        //std::cout <<  " Received:" << headerBuff << std::endl;
                        
                        memcpy(&thisHeader, headerBuff, HEADER_LENGTH);
                        if(thisHeader.tag!=0 && thisHeader.begin==10001)
                        {
                            if(thisHeader.totalBlock < thisHeader.indexOfBlock){ break; }
                            
                            isChecked_header = true;
                            contextBuff = new char[thisHeader.length];
                            
                            if(thisHeader.fileName != "" && !fileDataBegin )
                            {
                                fileDataBegin = true;
                                thisFileName = thisHeader.fileName;
                                thisFileType = thisHeader.fileType;
                            }
                        }
                        break;
                    }
                    
                    buf_Pointer++;
                }
            }
            if(isChecked_header)
            {
                fcntl(socketID, F_SETFL,  O_NONBLOCK);
                if (read(socketID, &contextBuff[buf_Context_Pointer], thisHeader.length) <= 0)
                {
                    if (errno == EWOULDBLOCK){ break; }
                    else{ break; }
                }
                //std::cout <<  "Received From "<< key << " : "<< contextBuff << std::endl;
                
                ////call
                if(!fileDataBegin) didGetMessage(contextBuff, thisHeader);
                ////
                
                if(fileDataBegin && thisFileName == thisHeader.fileName && thisFileType == thisHeader.fileType)
                {
                    
                    fileContextList.push_back(RecivedData(thisHeader,contextBuff));
                    
                    if(thisHeader.indexOfBlock == thisHeader.totalBlock)
                    {
                        int64_t blockCount = thisHeader.totalBlock;
                        int64_t thisfileLength;
                        if(blockCount > 2)
                        {
                            thisfileLength = FILE_BUFFER_SIZE * ( blockCount -1 );
                            thisfileLength += fileContextList.at(blockCount-1).header.length;
                        }
                        else
                        {
                            thisfileLength = thisHeader.length;
                        }
                        char* fileDataTotal = new char[thisfileLength];
                        
                        int64_t filePostionPointer = 0;
                        
                        LOOP(thisHeader.totalBlock)
                        {
                            //Index error
                            if(fileContextList.at(ii).header.indexOfBlock != (ii +1))
                            {
                                std::cout <<  "Recived file index error:" << std::endl;
                                thisFileName = "";
                                fileDataBegin = false;
                                thisFileType = FileTypes::NONE;
                                fileContextList.clear();
                                break;
                            }
                            
                            
                            //fileDataTotal += fileContextList.at(ii).data;
                            memcpy(fileDataTotal+filePostionPointer, fileContextList.at(ii).data, fileContextList.at(ii).header.length);
                            
                            filePostionPointer += fileContextList.at(ii).header.length;
                        }
                        ////call
                        Header outHeader = Header(thisHeader.tag,thisfileLength,1,1,"",thisHeader.fileType);
                        didGetFile(fileDataTotal, outHeader);
                        ////
                        
                        thisFileName = "";
                        fileDataBegin = false;
                        thisFileType = FileTypes::NONE;
                        fileContextList.clear();
                    }
                }
                else
                {
                    thisFileName = "";
                    fileDataBegin = false;
                    thisFileType = FileTypes::NONE;
                    fileContextList.clear();
                }
                

            }
            
        }
    }

	void YorkSocketServer::StopServer()
	{
		close(listenSocket);
	}

	void YorkSocketServer::SentMessageTo(int socketID, std::string words, int64_t tag, int64_t IOB, int64_t TOB)
	{
//		unsigned long sentSize = words.size();
//		char sentChar[sentSize + 1];
//        sentChar[sentSize] = endOfStream;
        char *sentChar = createBuffer(words, tag, TOB, IOB);
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
        
        char *sentChar = createBuffer(words, tag, TOB, IOB);
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
    
    
    
    

} /* namespace York */
