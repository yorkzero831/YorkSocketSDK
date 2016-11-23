/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_

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
	};

} 

#endif /* YORKNETWORK_H_ */
