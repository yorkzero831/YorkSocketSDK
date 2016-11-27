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
            std::this_thread::sleep_for(hearBeatC);
            
            bool isBegin_header                         = false;
            bool isChecked_header                       = false;
            bool isEndOfStream                          = false;
            
            Header thisHeader;
            
            long tag,blockNum,indexBlock,contentlength  = 0;
            
            size_t buf_Pointer                          = 0;
            size_t buf_Context_Pointer                  = 0;
            char headerBuff[HEADER_LENGTH]              = { 0 };
            char *contextBuff                           = nullptr;
            
            if(!isChecked_header)
            {
                while (buf_Pointer < HEADER_LENGTH)
                {
                    std::this_thread::sleep_for(hearBeatC);
                    fcntl(sockID, F_SETFL,  O_NONBLOCK);
                    if (read(sockID, &headerBuff[buf_Pointer], 1) <= 0)
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
                    if (buf_Pointer == HEADER_LENGTH-1)
                    {
                        //std::cout <<  " Received:" << headerBuff << std::endl;
                        
                        
                        memcpy(&thisHeader, headerBuff, HEADER_LENGTH);
                        if(thisHeader.tag!=0 && thisHeader.begin==10001)
                        {
                            if(thisHeader.totalBlock < thisHeader.indexOfBlock){break;}
                            isChecked_header = true;
                            contextBuff = new char[thisHeader.length];
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
                std::cout <<  " Received:" << contextBuff << std::endl;
                didGetMessage(contextBuff, thisHeader);
            }
            
        }
        

        
    }

} /* namespace York */
