/*
 * YorkNetwork.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
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
    
    char* YorkNetwork::createBuffer( char *preBuffer,  int64_t tag, int64_t numOfBlock,  int64_t indexOfBlock,  std::string fileName,  FileTypes fileType)
    {
        int64_t bufferLength;
        if(fileName == ""){ bufferLength = strlen(preBuffer); }
        else{ bufferLength =  FILE_BUFFER_SIZE; }
        
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock, fileName, fileType);
        
        memcpy(out, &header, HEADER_LENGTH);
        //int64_t ss = strlen(out);
        strcpy(out + HEADER_LENGTH, preBuffer);
        
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(std::string message,  int64_t tag, int64_t numOfBlock, int64_t indexOfBlock,  std::string fileName,  FileTypes fileType)
    {
        int64_t bufferLength;
        if(fileName == ""){ bufferLength = message.length(); }
        else{ bufferLength =  FILE_BUFFER_SIZE; }
        
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock, fileName, fileType);
        
        
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
    
    std::string YorkNetwork::getDirPath(std::string ins)
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
    
    YorkNetwork::FileTypes YorkNetwork::getFileType(std::string ins)
    {
        FileTypes out = YorkNetwork::FileTypes::NONE;
        
        if(ins == "png" || ins == "PNG" )
        {
            out = YorkNetwork::FileTypes::PNG;
        }
        else if(ins == "json" || ins == "JSON" )
        {
            out = YorkNetwork::FileTypes::JSON;
        }
        else if(ins == "jpg" || ins == "JPG" )
        {
            out = YorkNetwork::FileTypes::JPG;
        }
        else if(ins == "txt" || ins == "TXT" )
        {
            out = YorkNetwork::FileTypes::TXT;
        }
        
        return out;
    }
    
    std::string YorkNetwork::getStringByFileType(YorkNet::YorkNetwork::FileTypes ins)
    {
        std::string out = "";
        switch (ins)
        {
            case FileTypes::PNG :
                out = ".png";
                break;
            case FileTypes::JSON :
                out = ".json";
                break;
            case FileTypes::JPG :
                out = ".jpg";
                break;
            case FileTypes::TXT :
                out = ".txt";
                break;
            case FileTypes::NONE :
                out = "";
                break;
        }
        return out;
    }

} /* namespace York */
