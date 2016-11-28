/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_

#define DEFULT_PORT         10833
#define  MAX_BUFFER_SIZE    1000
#define HEADER_LENGTH       40
#define FILE_BUFFER_SIZE    984



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
            Header(const int64_t &Tag, const int64_t &Length, const int64_t &TB, const int64_t &IOB )
            {
                begin           = 10001;
                tag             = Tag;
                length          = Length;
                indexOfBlock    = IOB;
                totalBlock      = TB;
            }
            Header()
            {
                begin           = 10001;
                tag             = -1;
                length          = -1;
                indexOfBlock    = -1;
                totalBlock      = -1;
            }
        };
		void ShowErrorMessage(ErrorMessage message);
        
        char endOfStream    = '\0';
        
        const std::chrono::milliseconds hearBeatC = std::chrono::milliseconds(10);
        
        static char* createBuffer(char *preBuffer, int64_t tag, int64_t numOfBlock = 1, int64_t indexOfBlock = 1);
        static char* createBuffer(std::string message, int64_t tag , int64_t numOfBlock = 1, int64_t indexOfBlock = 1);
        static size_t getFileSize(const std::string& fileName);
        
        virtual void didGetMessage(const char *inMessage,const Header &header){};
        
//    private:
        
        static char* intToChar(const int64_t input);
        static int64_t charToInt(const char* input);
        static int64_t charToInt(const char* input, int beginP, int endP);
        
        static std::string getDirPath(std::string ins);
        
	};
    
    

} 

#endif /* YORKNETWORK_H_ */
