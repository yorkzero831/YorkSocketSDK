/*
 * YorkSocketClient.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#include "YorkSocketClient.h"


namespace YorkNet {

	YorkSocketClient::YorkSocketClient() {
		// TODO Auto-generated constructor stub

	}

	YorkSocketClient::~YorkSocketClient() {
		// TODO Auto-generated destructor stub
	}
    
    void YorkSocketClient::connectTo(std::string ip, int port)
    {
        ipAddr = ip;
        portNo = port;
        struct sockaddr_in server_addr;
        struct hostent     *server;
        
        char buffer[MAX_BUFFER_SIZE];
        sockID = socket(AF_INET, SOCK_STREAM, 0);
        if(sockID < 0)
        {
            std::cout<< "Error open socket"<< std::endl;
            return;
        }
        
        server = gethostbyname(ip.c_str());
        
        if(server == NULL)
        {
            std::cout<< "Can not get server on"<< ip << std::endl;
            return;
        }
        
        bzero((char *) &server_addr, sizeof(server_addr));
        
        server_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(portNo);
        
        if(connect(sockID, (struct sockaddr *) &server_addr, sizeof(server_addr))<0)
            std::cout<< "Error connecting to "<< ip <<" at port "<< portNo<< std::endl;
        
        waitMesThread = std::thread(&YorkNet::YorkSocketClient::readFromServer, this);
        waitMesThread.detach();
        
        cmdSysThread = std::thread(&YorkNet::YorkSocketClient::commandSystem, this);
        cmdSysThread.detach();
        
        while (true)
        {
           std::this_thread::sleep_for(hearBeatC);
        }
        
    }
    
    void YorkSocketClient::commandSystem()
    {
        std::cout<<"Input your command"<<std::endl;
        while (true)
        {
            std::this_thread::sleep_for(hearBeatC);
            
            std::string input = "";
            std::cin >> input;
            if(input == "sentMessage")
            {
                std::cout << "Input Message" << std::endl;
                std::string mess = "";
                std::cin >> mess;
                
                writeToServer(mess,1);
            }
            else if(input == "sentFile")
            {
                sentFileToServer("1", "png");
            }
            input = "";
        }
    }
    
    void YorkSocketClient::writeToServer(std::string message, int64_t tag, int64_t IOB, int64_t TOB)
    {
        char *sentChar = createBuffer(message, tag, TOB, IOB);
        int64_t size = message.length();
        
        //strcpy(sentChar, words.c_str());
        if (send(sockID, sentChar, size + HEADER_LENGTH, 0) == -1) {
            perror("Send error！");
        }
    }
    
    void YorkSocketClient::readFromServer()
    {
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
                    fcntl(sockID, F_SETFL,  O_NONBLOCK);
                    if (read(sockID, &headerBuff[buf_Pointer], 1) <= 0)
                    {
                        if (errno == EWOULDBLOCK){ break; }
                        else{ break; }
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
                fcntl(sockID, F_SETFL,  O_NONBLOCK);
                if (read(sockID, &contextBuff[buf_Context_Pointer], thisHeader.length) <= 0)
                {
                    if (errno == EWOULDBLOCK){ break; }
                    else{ break; }
                }
                std::cout <<  " Received:" << contextBuff << std::endl;
                
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
    
    void YorkSocketClient::sentFileToServer(std::string fileName, std::string fileType)
    {
        std::string filePath       = fileName + "." + fileType;
        std::string fileAbslutPath = getDirPath(filePath);
        
        // count fileBlock sent
        int64_t fileSize = getFileSize(fileAbslutPath);
        
        char *buffer = new char[FILE_BUFFER_SIZE];
        
        size_t fileBlockTotal = fileSize/FILE_BUFFER_SIZE;
        if(fileSize%FILE_BUFFER_SIZE > 0)
            fileBlockTotal++;
        
        FileTypes fileTypeT = getFileType(fileType);
        
//        std::ifstream ifs;
//        ifs.open(fileAbslutPath , std::ios::binary);
//        char testBuf[FILE_BUFFER_SIZE];
//        
//        ifs.read(testBuf, FILE_BUFFER_SIZE);
//        std::cout << "content: " << testBuf << std::endl;
        
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
            
            std::cout << "file_block_length: " << file_block_length << std::endl;
            std::cout << "content1: " << buffer << std::endl;
            
            char *sentChar = createBuffer(buffer, 2, fileBlockTotal, thisBlockNum, fileName, fileTypeT);
            int64_t sentLenth = strlen(buffer);
            
            if(send(sockID, sentChar, sentLenth + HEADER_LENGTH, 0) < 0)
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
