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
        StopServer();
	}
	

	void YorkSocketServer::StartServer(int portNum )
	{
        hostType = HostType::SERVER;
        
		//clients = std::map<std::string, int>();
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
        std::this_thread::sleep_for(heartBeatC);
        
        _fileList = getFileListFromData(getFileListDataFormFile());
		
        while (true) {
            std::this_thread::sleep_for(heartBeatC);
        }
		
	}
    
    void YorkSocketServer::commandSystem()
    {
        //while (true)
        //{
        std::this_thread::sleep_for(heartBeatC);
        
            std::string input = "";
            std::cin >> input;
            if (input == "clientCount")
            {
                //std::cout << clients.size() << std::endl;
                std::cout << clientSockets.size() << std::endl;
            }
            else if (input == "listClient")
            {
//                std::map<std::string, int>::iterator it;
//                for (it = clients.begin(); it != clients.end(); it++)
//                {
//                    std::cout << "ADDR:" << it->first << " ID:" << it->second << std::endl;
//                }
                std::map<int, std::string>::iterator it;
                for (it = clientSockets.begin(); it != clientSockets.end(); it++)
                {
                    std::cout << "ADDR:" << it->second << " ID:" << it->first << std::endl;
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
            else if(input == "sentFileTest")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                //sentFileToSocket(id, "1", "json");
                while (1) {
                    std::this_thread::sleep_for(heartBeatC*100);
                    sentFileToSocket(id, "1", "json", SentingFile());
                }
            }
            else if(input == "sentFile")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                sentFileToSocket(id, "2", "json",SentingFile());
            }
            else if(input == "sentFiles")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                LOOP(4)
                {
                    sentFileToSocket(id, "2", "json",SentingFile());
                }
            }
            else if(input == "sentFL")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                //sentFileToSocket(id, "2", "json",SentingFile());
                sentFileRequestToSocket(id, getDataFromFileList(_fileList), _fileList->size(), HeaderType::FILE_REQUEST_NEED_TO_SEND);
            }
            
        commandSystem();
        //}
    }



	void YorkSocketServer::runServer ()
	{
		
		while (1)
		{
            std::this_thread::sleep_for(heartBeatC);
            
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
                //clients.insert(std::pair<std::string, int>(ctrr, clientSocket));
                clientSockets.insert(std::pair<int, std::string>(clientSocket, ctrr));
                fileNeedToDoList.insert(std::pair<int, FileNeedToDo>(clientSocket,FileNeedToDo()));
				//if (!fork())
				{
                    //sentFileToSocket(clientSocket, "2", "json",SentingFile());
					//SentMessageTo(clientSocket, "You are In!!",1);
					//exit(0);
				}
                
                std::thread* waitingMessage = new std::thread(&YorkNet::YorkSocketServer::ListenToClient, this, clientSocket, ctrr);
                waitingMessageThreads.insert(std::pair<int, std::thread*>(clientSocket, waitingMessage));
                waitingMessage->detach();
                
			}
			
		}
	}
    
    void YorkSocketServer::ListenToClient(int socketID, std::string key)
    {
        int *checkedFileConformer                   = 0;
        bool doneRecieved                           = false;
        bool doneSend                               = false;
        while (1)
        {
            std::this_thread::sleep_for(heartBeatC);
            CheckerHeader checkHeader;
            size_t buf_Pointer                          = 0;
            char chectHeaderBuff[CHECKER_HEADER_LENGTH] = { 0 };
            
            while (buf_Pointer < CHECKER_HEADER_LENGTH)
            {
                fcntl(socketID, F_SETFL,  O_NONBLOCK);
                if (read(socketID, &chectHeaderBuff[buf_Pointer], 1) <= 0)
                {
                    if (errno == EWOULDBLOCK){ break; }
                    else
                    {
                        std::cout << key << " Removed" << std::endl;
                        clientSockets.erase(socketID);
                        close(socketID);
                        delete waitingMessageThreads[socketID];
                        waitingMessageThreads.erase(socketID);
                        fileNeedToDoList.erase(socketID);
                        clientFileRequestStatus.erase(socketID);
                        return;
                    }
                }
                
                size_t error;
                
                if (buf_Pointer == CHECKER_HEADER_LENGTH-1)
                {
                    memcpy(&checkHeader, chectHeaderBuff, CHECKER_HEADER_LENGTH);
                    if(checkHeader.begin != 10001){ break; }
                    switch (checkHeader.headerType)
                    {
                        case HeaderType::MESSAGES_TYPE :
                        {
                            error = readMessageFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_TYPE :
                        {
                            error = readFileFromSocket2(socketID, checkedFileConformer);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_CONFORMER :
                        {
                            error = readFileConformerFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_LIST :
                        {
                            error = readFileListFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::COMMAND_TYPE :
                        {
                            error = readCommandFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_REQUEST_NEED_TO_SEND :
                        {
                            error = readFileRequestFromSocket(socketID, FILE_REQUEST_NEED_TO_SEND);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_REQUEST_NEED_TO_RECIEVE :
                        {
                            error = readFileRequestFromSocket(socketID, FILE_REQUEST_NEED_TO_RECIEVE);
                            getError(error);
                            break;
                        }
                            
                            
                        default:
                            break;
                    }
                }
                buf_Pointer ++;
            }
        }

    }
    
	void YorkSocketServer::StopServer()
	{
		close(listenSocket);
        waitMesThread.~thread();
        connectThread.~thread();
        cmdSysThread.~thread();
        std::map<int, std::string>::iterator itor;
        for (itor = clientSockets.begin(); itor != clientSockets.end(); itor++)
        {
            int thisSocketID = itor->first;
            close(thisSocketID);
            waitingMessageThreads[thisSocketID]->~thread();
            delete waitingMessageThreads[thisSocketID];
        }
        
        
	}

	void YorkSocketServer::SentMessageTo(int socketID, std::string words, int64_t tag, int64_t IOB, int64_t TOB)
	{
        char *sentChar = createBufferForMessage(words);
        int64_t size = words.length();
        
		//strcpy(sentChar, words.c_str());
        fcntl(socketID, F_SETFL, O_NONBLOCK);
		if (send(socketID, sentChar, size + HEADER_LENGTH, 0) == -1) {
			perror("Send error！");

            clientSockets.erase(socketID);
		}
		//close(socketID);
	}

	void YorkSocketServer::SentMessageToALL(std::string words, int64_t tag, int64_t IOB, int64_t TOB)
	{
        std::vector<int> needToDelete =  std::vector<int>();
        
        char *sentChar = createBufferForMessage(words);
        int64_t size = words.length();

        std::map<int, std::string>::iterator it;
        for (it = clientSockets.begin(); it != clientSockets.end(); it++)
        {
            fcntl(it->first, F_SETFL, O_NONBLOCK);
            if (send(it->first, sentChar, size + HEADER_LENGTH, 0) == -1)
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
            clientSockets.erase(needToDelete.at(i));
        }
	}
    
    void YorkSocketServer::didGetFileList(std::map<std::string, FileListOne>* ins, const int &socketID)
    {
        clientFileRequestStatus.insert(std::pair<int, FileRequestStatus>(socketID, FileRequestStatus()));
        std::cout << "Get File List" << std::endl;

        
        std::map<std::string, FileListOne>* localFileList = new std::map<std::string, FileListOne>(*_fileList) ;
        
        FileNeedToDo thisSocketNeedToDo =  fileNeedToDoList[socketID];
        std::map<std::string, FileListOne>::iterator itor;
        for (itor = _fileList->begin(); itor != _fileList->end(); itor++)
        {
            int inVersion = ins->operator[](itor->first).version;
            int version   = itor->second.version;
            if(inVersion)
            {
                //
                if(inVersion < version)
                {
                    thisSocketNeedToDo.needToSend.push_back(itor->second);
                    //tempList.erase(itor->first);
                    
                }
                else if(inVersion > version)
                {
                    thisSocketNeedToDo.needToReceive.push_back(itor->second);
                    itor->second.version = inVersion;
                    //tempList.erase(itor->first);
                }
                ins->erase(itor->first);
            }
            else
            {
                thisSocketNeedToDo.needToSend.push_back(itor->second);
            }
            
        }
        
        for (itor = ins->begin(); itor != ins->end(); itor++)
        {
            thisSocketNeedToDo.needToReceive.push_back(itor->second);
            localFileList->insert(std::pair<std::string, FileListOne>(itor->first,itor->second));
        }
        
        std::cout << "Compare FileList Finished" << std::endl;
        
        //Server need to receive
        if(thisSocketNeedToDo.needToReceive.size() != 0)
        {
            //const char* saveData = getDataFromFileList(localFileList);
            
            //int64_t length = strlen(saveData);
            //saveFile(saveData, "fileList", FileTypes::TXT, length);
            
            const char* sentData = getDataFromFileList(thisSocketNeedToDo.needToReceive);
            
            sentFileRequestToSocket(socketID, sentData, thisSocketNeedToDo.needToReceive.size(), HeaderType::FILE_REQUEST_NEED_TO_SEND);
            
        }
        else
        {
            clientFileRequestStatus[socketID].recivedDone = true;
        }
        
        std::this_thread::sleep_for(heartBeatC);
        
        if(thisSocketNeedToDo.needToSend.size() != 0)
        {
            std::cout << "Will sent request" << std::endl;
            
            //thisSocketNeedToDo.needToSend.push_back(FileListOne("fileList",FileTypes::TXT,0));
            std::this_thread::sleep_for(heartBeatC);
            const char* sentData = getDataFromFileList(thisSocketNeedToDo.needToSend);
            sentFileRequestToSocket(socketID, sentData, thisSocketNeedToDo.needToSend.size(), HeaderType::FILE_REQUEST_NEED_TO_RECIEVE);
            
            
            LOOP(thisSocketNeedToDo.needToSend.size())
            {
                std::this_thread::sleep_for(heartBeatC);
                sentFileToSocket(socketID, thisSocketNeedToDo.needToSend.at(ii).name, getStringByFileType(thisSocketNeedToDo.needToSend.at(ii).type) ,SentingFile());
                
            }
            
            std::this_thread::sleep_for(heartBeatC);
            clientFileRequestStatus[socketID].requestDone = true;
        }
        else
        {
            clientFileRequestStatus[socketID].requestDone = true;
        }
        
        if(thisSocketNeedToDo.needToSend.size() == 0 && thisSocketNeedToDo.needToReceive.size() == 0)
        {
            std::cout << "Nothing Changed" << std::endl;
            std::this_thread::sleep_for(heartBeatC);
            sentCommandToSocket(socketID, CommandTypes::FILE_NO_NEED_CHANGE);
        }
        else
        {
            std::thread waitForRequestCompate(&YorkNet::YorkSocketServer::waitForRequestStatus, this, socketID, localFileList);
            waitForRequestCompate.detach();
        }
        
        
    }
    
    void YorkSocketServer::waitForRequestStatus(const int& socketID, std::map<std::string, FileListOne>*list)
    {
        while (1)
        {
            std::this_thread::sleep_for(heartBeatC);
            
            FileRequestStatus one = clientFileRequestStatus[socketID];
            if(waitingMessageThreads.find(socketID)== waitingMessageThreads.end())
            {
                return;
            }
            if(one.requestDone == true && one.recivedDone == true)
            {
                sentCommandToSocket(socketID, CommandTypes::FILE_NO_NEED_CHANGE);
                _fileList = list;
                const char* saveData = getDataFromFileList(list);
                int64_t length = strlen(saveData);
                
                saveFile(saveData, "fileList", FileTypes::TXT, length);
                
                clientFileRequestStatus.erase(socketID);
                return;
            }
        }
    }
    void YorkSocketServer::didGetCommand(const YorkNet::YorkNetwork::CommandHeader &inCmd, const int &socketID)
    {
#ifdef GET_COMMAND_DEBUG
        std::cout << "Got Command By ID "<< inCmd.cmd << std::endl;
#endif
        if(inCmd.cmd == CommandTypes::FILE_RECIECE_COMPLETED)
        {
            clientFileRequestStatus[socketID].recivedDone = true;
        }
    }
    
    
    

} /* namespace York */
