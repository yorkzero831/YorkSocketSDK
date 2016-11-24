/*
 * YorkSocketClient.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#include "YorkSocketClient.h"

#define MAX_BUFFER_SIZE 512

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
           
        }
        
    }
    
    void YorkSocketClient::commandSystem()
    {
        std::cout<<"Input your command"<<std::endl;
        while (true)
        {
            std::string input = "";
            std::cin >> input;
            if(input == "sentMessage")
            {
                std::cout << "Input Message" << std::endl;
                std::string mess = "";
                std::cin >> mess;
                
                writeToServer(mess);
            }
            input = "";
        }
    }
    
    void YorkSocketClient::writeToServer(std::string message)
    {
        int error;
        unsigned long sentSize = message.size();
        char sentChar[sentSize + 1];
        sentChar[sentSize] = endOfStream;
        
        strcpy(sentChar, message.c_str());
        if ((error = write(sockID,sentChar,sentSize) < 0))
        {
            std::cout<< "Error sent" << std::endl;
        }
    }
    
    void YorkSocketClient::readFromServer()
    {

        while (1)
        {
            bool isChecked_tag = false;
            bool isChecked_blockNum = false;
            bool isChecked_length = false;
            bool isChecked_indexBlock = false;
            int tag,blockNum,indexBlock,contentlength = 0;
            
            size_t buf_Pointer = 0;
            char thisBuf[MAX_BUFFER_SIZE] = { 0 };
            char *contentBuff = nullptr;
            while (buf_Pointer < MAX_BUFFER_SIZE)
            {
                fcntl(sockID, F_SETFL,  O_NONBLOCK);
                if (read(sockID, &thisBuf[buf_Pointer], 1) <= 0)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        //std::cout << "nothing" << std::endl;
                        break;;
                    }
                    else
                    {
                        break;
                    }
                }
                
                
//                if (buf_Pointer > 0 && length == buf_Pointer)
//                {
//                    std::cout << " Received:" << thisBuf << std::endl;
//                    break;
//                }
                
                if(buf_Pointer == (HEADER_LENGTH -1))
                {
                    tag = YorkNetwork::charToInt(thisBuf,0,8);
                    if(tag>=0)
                        isChecked_tag = true;
                    
                    blockNum = YorkNetwork::charToInt(thisBuf,8,16);
                    if(blockNum>=0)
                        isChecked_blockNum = true;
                    
                    indexBlock = YorkNetwork::charToInt(thisBuf,16,24);
                    if(indexBlock>=0)
                        isChecked_indexBlock = true;
                    
                    contentlength = YorkNetwork::charToInt(thisBuf,24,32);
                    if(contentlength>=0)
                    {
                        //std::cout<< contentlength << std::endl;
                        contentBuff = new char[contentlength];
                        isChecked_length = true;
                    }
                    
                }
                
                
                if(isChecked_tag && isChecked_blockNum && isChecked_length && isChecked_indexBlock && buf_Pointer >=HEADER_LENGTH )
                {
                    contentBuff[buf_Pointer-HEADER_LENGTH] = thisBuf[buf_Pointer];
                    if((buf_Pointer - HEADER_LENGTH +1) == contentlength)
                    {
                        std::cout << "Received:" << contentBuff << std::endl;
                        
                        break;
                    }
                }
                
                
                
                buf_Pointer++;
            }
        }
        

        
    }

} /* namespace York */
