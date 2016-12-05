/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
 */

#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_

#define DEFULT_PORT             10832
#define  MAX_BUFFER_SIZE        2120
#define HEADER_LENGTH           72
#define FILE_BUFFER_SIZE        100000
#define CHECKER_HEADER_LENGTH   12
#define MESSAGE_HEADER_LENGTH   16
#define SENTING_FILE_H_LENGTH   24

#define TIMEOUT                 5000



#define LOOP(n) for(long ii = 0; ii < n; ++ ii)
#define DISLOOP(n) for(long ii =n-1; ii >= 0; -- ii)
#define DELETE_CHARS_VECTOR(_VEC) for(long ii = 0; ii < _VEC.size(); ++ ii){ delete[] _VEC.at(ii).data; }

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
#include <map>


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
        
        enum HeaderType
        {
            CHECKER_TYPE,
            MESSAGES_TYPE,
            FILE_TYPE,
            FILE_CONFORMER
        };
        
        enum FileTypes
        {
            NONE,
            JSON,
            JPG,
            PNG,
            TXT
        };
        
        enum FileState
        {
            TRANSLATE_SUCCESS
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
        
        struct CheckerHeader
        {
            int64_t begin;
            HeaderType headerType;
            CheckerHeader(const HeaderType &inType)
            {
                begin = 10001;
                headerType = inType;
            }
            CheckerHeader()
            {
                begin = 10001;
                headerType = HeaderType::CHECKER_TYPE;
            }
        };
        
        struct MessageHeader
        {
            int64_t begin;
            int64_t length;
            MessageHeader(const int &len)
            {
                begin  = 1001;
                length = len;
            }
            MessageHeader()
            {
                begin  = -1;
                length = -1;
            }
        };
        
        struct FileConformerHeader
        {
            int64_t begin;
            int64_t indexOfFileBlock;
            FileConformerHeader(const int &IOB)
            {
                begin            = 1001;
                indexOfFileBlock = IOB;
            }
            FileConformerHeader()
            {
                begin            = -1;
                indexOfFileBlock = -1;
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
        
        struct SentingFile
        {
            int64_t begin;
            int64_t fileID;
            int64_t blockIndex;
            
            SentingFile(const int64_t &Id ,const int64_t &i)
            {
                begin      = 10001;
                fileID     = Id;
                blockIndex = i;
                //blockCount = c;
            }
            
            SentingFile()
            {
                begin      = 10001;
                fileID     = -1;
                blockIndex = -1;
                //blockCount = c;
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
            RecivedData()
            {
                header = Header();
                data   = nullptr;
            }
            
            ~RecivedData()
            {
                data = new char[0];
                delete[] data;
            }
        };
        
		void ShowErrorMessage(ErrorMessage message);
        
        char endOfStream    = '\0';
        
        const std::chrono::milliseconds hearBeatC = std::chrono::milliseconds(10);
        
        //Ceate Buffer For FileType
        char* createBuffer( char *preBuffer, int64_t tag, int64_t numOfBlock = 1,  int64_t indexOfBlock = 1, std::string fileName = "",  FileTypes fileType = FileTypes::NONE, int64_t fileLength = -1);
      
        //Ceate Buffer For MessageType
        char* createBuffer(const std::string &message);
        
        //Ceate Buffer For sentingFile
        char* createBuffer(const SentingFile &thisSentingFile);
        
        
        size_t getFileSize(const std::string& fileName);
        
        void sentFileToSocket(int socketID, std::string fileName, std::string fileType);
        
        virtual void readFromSocket(const int &socketID);
        
        virtual int readMessage(const int &socketID);
        
        virtual int readFile(const int &socketID, int *indexOfCheckedBlock);
        
        virtual int readFileConformer(const int &socketID);
        
        virtual void didGetFile(const char *inMessage,const Header &header);
        
        virtual void didGetMessage(const char *inMessage){std::cout<< inMessage <<std::endl;};
        
        void getError(size_t error);
        
    private:
        
        int64_t _fileID = 0;
        
        std::map<std::string, SentingFile> _sentingFiles = std::map<std::string, SentingFile>();
        std::map<std::string, SentingFile> _recivingFiles = std::map<std::string, SentingFile>();
        
        std::string getDirPath(std::string ins);
        
        FileTypes getFileType(std::string ins);
        
        std::string getStringByFileType(FileTypes ins);
        
        
        
	};
    
    

}

#endif /* YORKNETWORK_H_ */
