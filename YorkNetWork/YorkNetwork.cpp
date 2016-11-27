/*
 * YorkNetwork.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#include "YorkNetwork.h"


namespace YorkNet {

	YorkNetwork::YorkNetwork() {
		// TODO Auto-generated constructor stub

	}

	YorkNetwork::~YorkNetwork() {
		// TODO Auto-generated destructor stub
	}

	void YorkNetwork::ShowErrorMessage(ErrorMessage message)
	{
		std::cout << message.errorWords << " WITH ERROMESSAGE : " << message.details<<std::endl;
	}
    
    char* YorkNetwork::createBuffer(char *preBuffer, int64_t tag, int64_t numOfBlock, int64_t indexOfBlock)
    {
        int64_t bufferLength    = strlen(preBuffer);
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock);
        
        memcpy(out, &header, HEADER_LENGTH);
        //int64_t ss = strlen(out);
        strcpy(out + HEADER_LENGTH, preBuffer);
        
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(std::string message, int64_t tag, int64_t numOfBlock, int64_t indexOfBlock)
    {
        int64_t bufferLength    = message.length();
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock);
        
        
        memcpy(out, &header, HEADER_LENGTH);
        
        const char *messageToChar = message.c_str();
        
        strcpy(out + HEADER_LENGTH, messageToChar);
        
        
        return out;

    }

    
    char* YorkNetwork::intToChar(const int64_t input)
    {
        char *outPut = new char[8];
        memcpy(outPut, &input, 8);
        
        LOOP(8)
        {
            if(outPut[ii] == '\0')
            {
                outPut[ii] = '\x02';
            }
        }
        
        return outPut;
    }
    
    int64_t YorkNetwork::charToInt(const char *input)
    {
        int64_t n = 0;
        memcpy(&n, input, 8);
        
        return n;
    }
    
    int64_t YorkNetwork::charToInt(const char *input, int beginP, int endP)
    {
        if( (endP - endP) != 8 )
            return -1;
        char tempChar[8];
        LOOP(8)
        {
            tempChar[ii] = input[ii + beginP];
        }
        
        int64_t n = 0;
        memcpy(&n, tempChar, 8);
        return n;
        
    }
    
    size_t YorkNetwork::getFileSize(const std::string& fileName)
    {
        struct stat st;
        if(stat(fileName.c_str(), &st) != 0) {
            return 0;
        }
        return st.st_size;
    }

} /* namespace York */
