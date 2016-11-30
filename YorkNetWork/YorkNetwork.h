/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
 */

#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_

#define DEFULT_PORT         10832
#define  MAX_BUFFER_SIZE    2120
#define HEADER_LENGTH       72
#define FILE_BUFFER_SIZE    4096



#define LOOP(n) for(long ii = 0; ii < n; ++ ii)
#define DISLOOP(n) for(long ii =n-1; ii >= 0; -- ii)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
//#include <pthread.h>
#include <netinet/in.h> 
#include <netdb.h>  
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>

#include <thread>
#include <vector>
#include <sys/mman.h>
#include <fstream>


//using namespace::std;

namespace YorkNet {

	
	class YorkNetwork {
	public:
		YorkNetwork();
	    ~YorkNetwork();
		enum ErrorWord {
			CANNOTLISITENPORT,
			CANNOTBLINDSERVER,
			CANNOTCREATESOCKT,
			CANNOTACCETTCLIENT
		};
        
        enum FileTypes
        {
            NONE,
            JSON,
            JPG,
            PNG,
            TXT
        };
        
		struct ErrorMessage
		{
			ErrorWord errorWords;
			std::string details;
			ErrorMessage(ErrorWord words, std::string det)
			{
				errorWords	 = words;
				details		 = det;
			}
		};
        
        
        struct Header
        {
            int64_t begin;
            int64_t tag;
            int64_t length;
            int64_t indexOfBlock;
            int64_t totalBlock;
            std::string fileName;
            FileTypes fileType;
            
            Header(const int64_t &Tag, const int64_t &Length, const int64_t &TB, const int64_t &IOB , const std::string &fn, const FileTypes &ft)
            {
                begin           = 10001;
                tag             = Tag;
                length          = Length;
                indexOfBlock    = IOB;
                totalBlock      = TB;
                fileName        = fn;
                fileType        = ft;
                
            }
            Header(const int64_t &Tag, const int64_t &Length, const int64_t &TB, const int64_t &IOB)
            {
                begin           = 10001;
                tag             = Tag;
                length          = Length;
                indexOfBlock    = IOB;
                totalBlock      = TB;
                fileName        = "";
                fileType        = NONE;
                
            }
            Header()
            {
                begin           = 10001;
                tag             = -1;
                length          = -1;
                indexOfBlock    = -1;
                totalBlock      = -1;
                fileName        = "";
                fileType        = NONE;
            }
        };
        
        struct RecivedData
        {
            Header header;
            char*  data;
            RecivedData(Header h, char* d)
            {
                header  = h;
                data    = d;
            }
        };
        
		void ShowErrorMessage(ErrorMessage message);
        
        char endOfStream    = '\0';
        
        const std::chrono::milliseconds hearBeatC = std::chrono::milliseconds(10);
        
        char* createBuffer( char *preBuffer, int64_t tag, int64_t numOfBlock = 1,  int64_t indexOfBlock = 1, std::string fileName = "",  FileTypes fileType = FileTypes::NONE, int64_t fileLength = -1);
        
        char* createBuffer(std::string message, int64_t tag, int64_t numOfBlock = 1, int64_t indexOfBlock = 1, std::string fileName = "", FileTypes fileType = FileTypes::NONE, int64_t fileLength = -1);
        
        //char* createBuffer( char *preBuffer,  int64_t tag, int64_t numOfBlock = 1,  int64_t indexOfBlock = 1,  std::string fileName = "",  FileTypes fileType = FileTypes::NONE);
        
        size_t getFileSize(const std::string& fileName);
        
        void sentFileToSocket(int socketID, std::string fileName, std::string fileType);
        
        virtual void didGetFile(const char *inMessage,const Header &header);

        
        
        
        virtual void didGetMessage(const char *inMessage,const Header &header){std::cout<< inMessage <<std::endl;};
        
//    private:
        
        char* intToChar(const int64_t input);
        int64_t charToInt(const char* input);
        int64_t charToInt(const char* input, int beginP, int endP);
        
        std::string getDirPath(std::string ins);
        
        FileTypes getFileType(std::string ins);
        std::string getStringByFileType(FileTypes ins);
        
	};
    
    

} 

#endif /* YORKNETWORK_H_ */
