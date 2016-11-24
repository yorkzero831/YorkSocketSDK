/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_

#define DEFULT_PORT 10832
#define  MAX_BUFFER_SIZE 2048
#define HEADER_LENGTH 48
#define FILE_BUFFER_SIZE 2000

#define LOOP(n) for(int ii = 0; ii < n; ++ ii)
#define DISLOOP(n) for(int ii =n-1; ii >= 0; -- ii)

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
#include <pthread.h>
#include <netinet/in.h> 
#include <netdb.h>  
#include <fcntl.h>
#include <math.h>

#include <thread>


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
		void ShowErrorMessage(ErrorMessage message);
        
        char endOfStream = '\0';
        
        static char* createBuffer(char *preBuffer, int tag, int numOfBlock = 1, int indexOfBlock = 1);
        static char* createBuffer(std::string message, int tag , int numOfBlock = 1, int indexOfBlock = 1);
        static size_t getFileSize(const std::string& fileName);
        
//    private:
        static char* intToChar(const int input, int length = 0);
        static int charToInt(const char* input);
        static int charToInt(const char* input, int beginP, int endP);
	};

} 

#endif /* YORKNETWORK_H_ */
