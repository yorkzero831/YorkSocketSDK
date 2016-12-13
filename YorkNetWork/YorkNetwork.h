/*
 * YorkNetwork.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero8
 */



#ifndef YORKNETWORK_H_
#define YORKNETWORK_H_


//#define FILE_SENT_DEBUG         0;
//#define FILE_RECIEVE_DEBUG      0;
//#define GET_COMMAND_DEBUG       0;
//#define SENTMESSAGE_DEBUG       0;



#define DEFULT_PORT             10832
#define MAX_BUFFER_SIZE         2120
#define HEADER_LENGTH           72
#define FILE_BUFFER_SIZE        100000
#define CHECKER_HEADER_LENGTH   12
#define MESSAGE_HEADER_LENGTH   16
#define SENTING_FILE_H_LENGTH   24
#define FILES_LIST_LENGTH       24
#define COMMAND_HEADER_LENGTH   16

#define TIMEOUT                 5000

#define heartBeatC              std::chrono::milliseconds(10)



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
        
        enum HostType
        {
            NOTYPE,
            SERVER,
            CLIENT
        };
        
        enum HeaderType
        {
            CHECKER_TYPE,
            MESSAGES_TYPE,
            FILE_TYPE,
            FILE_CONFORMER,
            COMMAND_TYPE,
            FILE_LIST,
            FILE_REQUEST_NEED_TO_SEND,
            FILE_REQUEST_NEED_TO_RECIEVE
        };
        
        enum FileTypes
        {
            NONE,
            JSON,
            JPG,
            PNG,
            TXT
        };
        
        enum CommandTypes
        {
            NONE_COMMAND,
            FILE_NO_NEED_CHANGE,
            FILE_RECIECE_COMPLETED
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
        
        struct CommandHeader
        {
            int64_t      begin;
            CommandTypes cmd;
            CommandHeader()
            {
                begin = -1;
                cmd   = CommandTypes::NONE_COMMAND;
            }
            CommandHeader(const CommandTypes &inCmd)
            {
                begin = 10001;
                cmd   = inCmd;
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
                begin = -1;
                headerType = HeaderType::CHECKER_TYPE;
            }
        };
        
        struct MessageHeader
        {
            int64_t begin;
            int64_t length;
            MessageHeader(const int64_t &len)
            {
                begin  = 10001;
                length = len;
            }
            MessageHeader()
            {
                begin  = -1;
                length = -1;
            }
        };
        
        struct FileListHeader
        {
            int64_t begin;
            int64_t dataLength;
            int64_t count;
            FileListHeader()
            {
                begin       = -1;
                dataLength  = -1;
                count       = -1;
            }
            FileListHeader(const int64_t &l, const int64_t &c)
            {
                begin       = 10001;
                dataLength  = l;
                count       = c;
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
        
        struct YorkCommand
        {
            
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
                //data = new char[0];
                //delete[] data;
            }
        };
        
        struct FileListOne
        {
            std::string name;
            FileTypes   type;
            int         version;
            
            FileListOne()
            {
                name    = "";
                type    = FileTypes::NONE;
                version = -1;
            }
            FileListOne(std::string n, FileTypes t, int v)
            {
                name    = n;
                type    = t;
                version = v;
            }
        };
        
        struct OpeningFile
        {
            FILE* file;
            std::string fileName;
            int64_t indexCount;
            int64_t fileSize;
            FileTypes filetype;
            
            OpeningFile()
            {
                file        = nullptr;
                indexCount  = -1;
                fileSize    = -1;
            }
            OpeningFile(FILE* f, std::string fn, const int64_t& ic, const int64_t& fs, const FileTypes& ft)
            {
                file        = f;
                fileName    = fn;
                indexCount  = ic;
                fileSize    = fs;
                filetype    = ft;
            }
            
        };
        
        HostType hostType = HostType::NOTYPE;
        
        std::map<std::string, FileListOne>* _fileList        = new std::map<std::string, FileListOne>();
        
        
		void ShowErrorMessage(ErrorMessage message);
        
        char endOfStream    = '\0';
        
        //Ceate Buffer For FileType
        char* createBufferForFile( char *preBuffer, int64_t tag, int64_t numOfBlock = 1,  int64_t indexOfBlock = 1, std::string fileName = "",  FileTypes fileType = FileTypes::NONE, int64_t fileLength = -1);
      
        //Ceate Buffer For MessageType
        char* createBufferForMessage(const std::string &message);
        
        //Ceate Buffer For sentingFile
        char* createBufferForConformer(const SentingFile &thisSentingFile);
        
        char* createBufferForFileList(const FileListHeader &thisFRHeader, const char* fileRequestListData, const HeaderType &type);
        
        char* createBufferForCommand(const CommandTypes &cmd);
        
        size_t getFileSize(const std::string& fileName);
        
        //int sentFileToSocket(const int &socketID, std::string fileName, std::string fileType);
        
        int sentFileToSocket(const int &socketID, std::string fileName, std::string fileType, const SentingFile &fileConformerH);
        
        int sentFileListToSocket(const int &socketID, const char* fileRequestData, const int64_t &fileCount);
        
        int sentFileRequestToSocket(const int &socketID, const char* fileRequestData, const int64_t &fileCount, const HeaderType &type);
        
        int sentCommandToSocket(const int &socketID, const CommandTypes &cmd);
        
        virtual void readFromSocket(const int &socketID);
        
        virtual int readMessageFromSocket(const int &socketID);
        
        //virtual int readFileFromSocket(const int &socketID, int *indexOfCheckedBlock);
        
        virtual int readFileFromSocket2(const int &socketID, int *indexOfCheckedBlock);
        
        virtual int readFileConformerFromSocket(const int &socketID);
        
        virtual int readFileListFromSocket(const int &socketID);
        
        virtual int readFileRequestFromSocket(const int &socketID, const HeaderType &type);
        
        virtual int readCommandFromSocket(const int &socketID);
        
        virtual void didGetFileData(const char *inMessage,const Header &header);
        
        virtual void didGetFile(const Header &header);
        
        virtual void didGetMessage(const char *inMessage){std::cout<< inMessage <<std::endl;};
        
        void getError(size_t error);
        
        std::string getStringByFileType(FileTypes ins);
        
        virtual std::string getDirPath(std::string ins);
        
        void saveFile(const char *inMessage, const std::string &name, const FileTypes &fileType, const int64_t &size);
        
        
        
        virtual const char* getFileListDataFormFile();
        
        virtual std::map<std::string, FileListOne>* getFileListFromData(const char* ins);
        
        virtual const char* getDataFromFileList(std::map<std::string, FileListOne>* ins);
        
        virtual const char* getDataFromFileList(std::vector<FileListOne> ins);
        
        virtual void didGetFileList(std::map<std::string, FileListOne>* ins, const int &socketID){};
        
        virtual void didGetFileRequestList(std::map<std::string, FileListOne>* ins, const int &socketID, const HeaderType &type){};
        
        virtual void didGetCommand(const CommandHeader &inCmd, const int &socketID){};
        
        
    private:
        
        int64_t _fileID = 0;
        
        std::map<std::string, SentingFile> _sentingFiles    = std::map<std::string, SentingFile>();
        std::map<std::string, SentingFile> _recivingFiles   = std::map<std::string, SentingFile>();
        
        std::map<std::string, OpeningFile> _openingFiles    = std::map<std::string, OpeningFile>();
        
        std::map<std::string, std::vector<RecivedData>*> _recivingDatas = std::map<std::string, std::vector<RecivedData>*>();
        
        FileTypes getFileType(std::string ins);
        
        
	};
    
    

}

#endif /* YORKNETWORK_H_ */
