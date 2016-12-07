/*
 * YorkSocketClient.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
 */

#include "YorkSocketClient.h"


namespace YorkNet {

	YorkSocketClient::YorkSocketClient() {
		// TODO Auto-generated constructor stub

	}

	YorkSocketClient::~YorkSocketClient() {
		// TODO Auto-generated destructor stub
        disconnect();
	}
    
    void YorkSocketClient::connectTo(std::string ip, int port)
    {
        hostType = HostType::CLIENT;
        
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
        
        waitMesThread = std::thread(&YorkNet::YorkSocketClient::readFromSocket, this, sockID);
        waitMesThread.detach();
        
        cmdSysThread = std::thread(&YorkNet::YorkSocketClient::commandSystem, this);
        cmdSysThread.detach();
        
        while (!finishedFlag)
        {
           std::this_thread::sleep_for(hearBeatC);
        }
        
        //disconnect();
        //std::cout<<"END"<<std::endl;
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
                sentFileToSocket(sockID, "2", "json");
            }
            input = "";
        }
    }
    
    void YorkSocketClient::disconnect()
    {
        close(sockID);
        //waitMesThread.~thread();
        //cmdSysThread.~thread();
        finishedFlag = true;
    }
    
    void YorkSocketClient::writeToServer(std::string message, int64_t tag, int64_t IOB, int64_t TOB)
    {
        char *sentChar = createBuffer(message);
        int64_t size = message.length();
        
        //strcpy(sentChar, words.c_str());
        if (send(sockID, sentChar, size + HEADER_LENGTH, 0) == -1) {
            perror("Send error！");
        }
    }
    
    void YorkSocketClient::didGetFile(const YorkNet::YorkNetwork::Header &header)
    {
        //disconnect();
        std::cout<<"Got File "<<header.fileName<< getStringByFileType(header.fileType) << std::endl;
    }
    
    std::string YorkSocketClient::getDirPath(std::string ins)
    {
        std::string out;
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            out.append(cwd);
            out += "/file/";
            out += ins;
        }
        return out;
    }

    

} /* namespace York */
