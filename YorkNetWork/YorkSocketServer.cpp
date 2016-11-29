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
                
                SentFileTo(id, "1", "json");
            }
            else if(input == "sentFile1")
            {
                std::cout << "Input ClientID" << std::endl;
                int id = 0;
                std::cin >> id;
                
                SentFileTo(id, "1", "txt");
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
                std::cout <<  "Received From "<< key << " : "<< contextBuff << std::endl;
                
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
                        //char* fileDataTotal = new char[thisfileLength];
                        std::string fileDataTotal;
                        
                        int64_t totalFileLength = 0;
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
                            totalFileLength += fileContextList.at(ii).header.length;
                            fileDataTotal += fileContextList.at(ii).data;
                        }
                        ////call
                        Header outHeader = Header(thisHeader.tag,totalFileLength,1,1);
                        didGetFile(fileDataTotal.c_str(), outHeader);
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
    
    void YorkSocketServer::SentFileTo(int socketID, std::string fileName, std::string fileType)
    {
        std::string filePath       = fileName + "." + fileType;
        std::string fileAbslutPath = getDirPath(filePath);
        
        // count fileBlock sent
        int64_t fileSize = getFileSize(fileAbslutPath);

        char buffer[FILE_BUFFER_SIZE];
        
        size_t fileBlockTotal = fileSize/FILE_BUFFER_SIZE;
        if(fileSize%FILE_BUFFER_SIZE > 0)
            fileBlockTotal++;
        
     
        FileTypes fileTypeT = getFileType(fileType);
        
        FILE *fileR;
        
        if(fileTypeT == FileTypes::TXT || fileTypeT == FileTypes::JSON || fileTypeT == FileTypes::NONE)
        {
            fileR = std::fopen(fileAbslutPath.c_str(), "r");
        }
        else
        {
            fileR = std::fopen(fileAbslutPath.c_str(), "rb");
        }
        
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
            
            std::cout << thisBlockNum << " file_block_length: " << file_block_length << std::endl;
            //std::cout << "content: " << buffer << std::endl;
            
            char *sentChar = createBuffer(buffer, 2, fileBlockTotal, thisBlockNum, fileName, fileTypeT);
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
    
    void YorkSocketServer::didGetFile(const char *inMessage, const YorkNet::YorkNetwork::Header &header)
    {
        std::string fileName = "temp" + getStringByFileType(header.fileType);
        
        FILE *fileToWrite;
        if(header.fileType == FileTypes::TXT || header.fileType == FileTypes::JSON || header.fileType == FileTypes::NONE)
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "w+");
        }
        else
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "wb");
        }
        fwrite(inMessage, sizeof(char), header.length, fileToWrite);
        fclose(fileToWrite);
        std::cout<<"Finsh on save file"<< std::endl;
        
    }
    
    
    

} /* namespace York */
