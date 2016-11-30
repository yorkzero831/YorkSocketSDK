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
    
    char* YorkNetwork::createBuffer( char *preBuffer,  int64_t tag, int64_t numOfBlock,  int64_t indexOfBlock,  std::string fileName,  FileTypes fileType, int64_t fileLength)
    {
        int64_t bufferLength;
        if(fileLength == -1){ bufferLength = strlen(preBuffer); }
        else{ bufferLength =  fileLength; }
        
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock, fileName, fileType);
        
        memcpy(out, &header, HEADER_LENGTH);
        //int64_t ss = strlen(out);
        //strcpy(out + HEADER_LENGTH, preBuffer);
        memcpy(out + HEADER_LENGTH, preBuffer, bufferLength);
        //strncpy(out + HEADER_LENGTH, preBuffer, bufferLength);
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(std::string message,  int64_t tag, int64_t numOfBlock, int64_t indexOfBlock,  std::string fileName,  FileTypes fileType, int64_t fileLength)
    {
        int64_t bufferLength;
        if(fileLength == -1){ bufferLength = message.length(); }
        else{ bufferLength =  fileLength; }
        
        char *out               = new char[bufferLength + HEADER_LENGTH];
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock, fileName, fileType);
        
        
        memcpy(out, &header, HEADER_LENGTH);
        
        const char *messageToChar = message.c_str();
        
        //strcpy(out + HEADER_LENGTH, messageToChar);
        memcpy(out + HEADER_LENGTH, messageToChar, bufferLength);
        
        
        return out;

    }
    
    //Function called when sent file to socket
    void YorkNetwork::sentFileToSocket(int socketID, std::string fileName, std::string fileType)
    {
        std::string filePath       = fileName + "." + fileType;
        std::string fileAbslutPath = getDirPath(filePath);
        
        // count fileBlock sent
        int64_t fileSize = getFileSize(fileAbslutPath);
        
        char buffer[FILE_BUFFER_SIZE];
        
        size_t fileBlockTotal = fileSize/FILE_BUFFER_SIZE;
        if(fileSize%FILE_BUFFER_SIZE > 0)
            fileBlockTotal++;
        
        FileTypes fileTypeT = getFileType(fileType);
        
        FILE *fileR;
        
        if(fileTypeT == FileTypes::TXT || fileTypeT == FileTypes::JSON || fileTypeT == FileTypes::NONE)
        {
            fileR = std::fopen(fileAbslutPath.c_str(), "r");
        }
        else
        {
            fileR = std::fopen(fileAbslutPath.c_str(), "rb");
        }
        
        
        if(fileR == NULL)
        {
            fclose(fileR);
            std::cout << "Can not OpenFile "<< filePath << std::endl;
            return;
        }
        
        
        int thisBlockNum = 0;
        
        bzero(buffer, FILE_BUFFER_SIZE);
        int file_block_length = 0;
        
        while( (file_block_length = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, fileR)) > 0)
        {
            thisBlockNum ++;
            
            //std::cout << "file_block_length: " << file_block_length << std::endl;
            //std::cout << "content1: " << buffer << std::endl;
            
            int64_t blockLenth;
            if(thisBlockNum < fileBlockTotal){ blockLenth = FILE_BUFFER_SIZE;}
            else if (thisBlockNum == fileBlockTotal) { blockLenth = fileSize%FILE_BUFFER_SIZE ;}
            
            char *sentChar = createBuffer(buffer, 2, fileBlockTotal, thisBlockNum, fileName, fileTypeT, blockLenth);
            int64_t sentLenth = strlen(buffer);
            
            if(send(socketID, sentChar, blockLenth+HEADER_LENGTH, 0) < 0)
            {
                int couter = 0;
                bool fixed = false;
                while (couter <100)
                {
                    std::this_thread::sleep_for(hearBeatC);
                    couter++;
                    if(send(socketID, sentChar, blockLenth+HEADER_LENGTH, 0) < 0 )
                    {
                        std::cout<<"Fixing"<<std::endl;
                        continue;
                    }
                    else
                    {
                        fixed = true;
                        break;
                    }
                }
                
                if(!fixed)
                {
                    delete sentChar;
                    std::cout << "Error on sending file"  << std::endl;
                    break;
                }
                
            }
            
            delete sentChar;
            
            bzero(buffer, FILE_BUFFER_SIZE);
            
        }
        //SentMessageTo(socketID, "",1);
        
        fclose(fileR);
        std::cout << "File: "<< filePath<<" Transfer finished"  << std::endl;
    }
    
    //Fuction called when file did all recived
    void YorkNetwork::didGetFile(const char *inMessage, const YorkNet::YorkNetwork::Header &header)
    {
        std::string fileName = "temp" + getStringByFileType(header.fileType);
        
        FILE *fileToWrite;
        if(header.fileType == FileTypes::TXT || header.fileType == FileTypes::JSON || header.fileType == FileTypes::NONE)
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "w+");
        }
        else
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "wb");
        }
        fwrite(inMessage, sizeof(char), header.length, fileToWrite);
        fclose(fileToWrite);
        delete (inMessage);
        std::cout<<"Finsh on save file"<< std::endl;
        
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
