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
    
    char* YorkNetwork::createBuffer(char *preBuffer, int tag, int numOfBlock)
    {
        char *out = new char[MAX_BUFFER_SIZE];
        unsigned long length = strlen(preBuffer);
        
        if(length > MAX_BUFFER_SIZE - HEADER_LENGTH)
        {
            std::cout << "Buffer size over ranger" << std::endl;
            return out;
        }

        // create part of how much words does content have
        char *lengthChar = YorkNetwork::intToChar((int)length, 8);
        
        // create part of tag
        char *tagChar    = YorkNetwork::intToChar(tag, 8);
        
        // create part of numOfBlock
        char *numOfBlockCHar    = YorkNetwork::intToChar(numOfBlock, 8);
        
        char *pointer = out;
        
        // HeaderLenth =  tag + numOfBlock + length
        LOOP(8)
        {
            *pointer = tagChar[ii];
            pointer++;
        }
        
        LOOP(8)
        {
            *pointer = numOfBlockCHar[ii];
            pointer++;
        }
        
        LOOP(8)
        {
            *pointer = lengthChar[ii];
            pointer++;
        }
        
        LOOP(length)
        {
            *pointer = preBuffer[ii];
            pointer++;
        }
 
        
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(std::string message, int tag, int numOfBlock)
    {
        char *out = new char[MAX_BUFFER_SIZE];
        
        unsigned long length = message.length();
        
        if(length > MAX_BUFFER_SIZE - HEADER_LENGTH)
        {
            std::cout << "Buffer size over ranger" << std::endl;
            return out;
        }
        
        // create part of how much words does content have
        char *lengthChar = YorkNetwork::intToChar((int)length, 8);
        
        // create part of tag
        char *tagChar    = YorkNetwork::intToChar(tag, 8);
        
        // create part of numOfBlock
        char *numOfBlockCHar    = YorkNetwork::intToChar(numOfBlock, 8);
        
        char *pointer = out;
        
        // HeaderLenth =  tag + numOfBlock + length
        LOOP(8)
        {
            *pointer = tagChar[ii];
            pointer++;
        }
        
        LOOP(8)
        {
            *pointer = numOfBlockCHar[ii];
            pointer++;
        }
        
        LOOP(8)
        {
            *pointer = lengthChar[ii];
            pointer++;
        }
        
        LOOP(length)
        {
            *pointer = message.at(ii);
            pointer++;
        }
        
        
        
        return out;

    }
    
    char* YorkNetwork::intToChar(const int input, int length)
    {
        
        char s[32];
        int len = sprintf(s, "%d", input);
        char *output;
        
        if(length != 0)
        {
            if(length < len)
            {
                return output;
            }
            
            output = new char[length];
            int dist = length - len;
            
            LOOP(dist)
            {
                output[ii] = '0';
            }
            
            for (int i = dist; i < length; i++)
            {
                output[i] = s[i-dist];
            }
            
            
        }
        else
        {
            output = new char[len];
            DISLOOP(len)
            {
                output[ii] = s[ii];
            }
        }
        
        return output;
        
        
    }
    
    int YorkNetwork::charToInt(const char *input)
    {
        unsigned long length = strlen(input);
        int ruler = 1;
        int outNum = 0;
        DISLOOP((int)length)
        {
            int thisNum = input[ii] - '0';
            outNum += ( thisNum * ruler );
            ruler *= 10;
        }
        
        
        return outNum;
    }
    
    int YorkNetwork::charToInt(const char *input, int beginP, int endP)
    {
        int length = endP - beginP;
        
        if(length < 0) return 0;
        
        int ruler = 1;
        int outNum = 0;
        for(int i = endP-1; i>= beginP; i--)
        {
            int thisNum = input[i] - '0';
            outNum += ( thisNum * ruler );
            ruler *= 10;
        }
        return outNum;
        
        
    }

} /* namespace York */
