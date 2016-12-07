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
        
        char *out               = new char[bufferLength + HEADER_LENGTH + CHECKER_HEADER_LENGTH];
        
        CheckerHeader cekHeader = CheckerHeader(HeaderType::FILE_TYPE);
        
        int64_t tagTrue         = tag==0?1:tag;
        Header header           = Header(tagTrue, bufferLength, numOfBlock, indexOfBlock, fileName, fileType);
        
        memcpy(out, &cekHeader, CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &header, HEADER_LENGTH);
        //int64_t ss = strlen(out);
        //strcpy(out + HEADER_LENGTH, preBuffer);
        memcpy(out + CHECKER_HEADER_LENGTH + HEADER_LENGTH, preBuffer, bufferLength);
        //strncpy(out + HEADER_LENGTH, preBuffer, bufferLength);
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(const std::string &message)
    {
        int64_t bufferLength    = message.length();
        MessageHeader mHeader   = MessageHeader(bufferLength);

        CheckerHeader cekHeader = CheckerHeader(HeaderType::MESSAGES_TYPE);
        
        char *out               = new char[bufferLength + CHECKER_HEADER_LENGTH + MESSAGE_HEADER_LENGTH];
        
        memcpy(out, &cekHeader, CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &mHeader, MESSAGE_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH + MESSAGE_HEADER_LENGTH, message.c_str(), bufferLength);
        
        return out;
    }
    
    char* YorkNetwork::createBuffer(const SentingFile &thisSentingFile)
    {
        char *out = new char[CHECKER_HEADER_LENGTH + SENTING_FILE_H_LENGTH];
        CheckerHeader cekHeader = CheckerHeader(HeaderType::FILE_CONFORMER);
        
        memcpy(out, &cekHeader , CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &thisSentingFile, SENTING_FILE_H_LENGTH);
        
        return out;
    }
    
    //Function called when sent file to socket
    int YorkNetwork::sentFileToSocket(int socketID, std::string fileName, std::string fileType)
    {
        bool isFirstTime =true;
        
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
            return -1001;
        }
        
        
        int64_t thisBlockNum = 0;
        
        bzero(buffer, FILE_BUFFER_SIZE);
        int file_block_length = 0;

        
        _fileID++;
        //put this file into sentingFile list
        std::string fileUniName = "local_file_" + std::to_string(_fileID);
        
        
        while( (file_block_length = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, fileR)) > 0)
        {
            
            //std::this_thread::sleep_for(hearBeatC);
            
            //std::cout << "file_block_length: " << file_block_length << std::endl;
            //std::cout << "content1: " << buffer << std::endl;
            thisBlockNum ++;
            
            if(!isFirstTime)
            {
                while ( _sentingFiles[fileUniName].blockIndex - thisBlockNum != -1)
                {
                    std::this_thread::sleep_for(hearBeatC/10);
                }
            }
            else
            {
                SentingFile thisSentingFile = SentingFile(_fileID,-1);
                _sentingFiles.insert(std::pair<std::string,SentingFile>(fileUniName,thisSentingFile));
                isFirstTime = false;
            }
            
            
            
            int64_t blockLenth;
            if(thisBlockNum < fileBlockTotal){ blockLenth = FILE_BUFFER_SIZE;}
            else if (thisBlockNum == fileBlockTotal) { blockLenth = fileSize%FILE_BUFFER_SIZE ;}
            
            char *sentChar = createBuffer(buffer, _fileID, fileBlockTotal, thisBlockNum, fileName, fileTypeT, blockLenth);
            int64_t sentLenth = strlen(buffer);
            
            fcntl(socketID, F_SETFL,  O_NONBLOCK);
            if(send(socketID, sentChar, blockLenth + HEADER_LENGTH + CHECKER_HEADER_LENGTH, 0) < 0)
            {
                getError(errno);

                delete[] sentChar;
                std::cout << "Error on sending file"  << std::endl;
                break;
                
            }
            //std::cout <<  "===========Send "<< thisBlockNum<<":"<< fileBlockTotal<< std::endl;
            
            delete[] sentChar;
            
            bzero(buffer, FILE_BUFFER_SIZE);
            
            
            
        }
        //SentMessageTo(socketID, "",1);
        _sentingFiles.erase(fileUniName);
        
        fclose(fileR);
        std::cout << "File: "<< filePath<<" Transfer finished"  << std::endl;
        
        return 0;
    }
    
    void YorkNetwork::readFromSocket(const int &socketID)
    {
        int *checkedFileConformer                   = 0;
        std::string *fileName                       = new std::string("");
        
        while (1)
        {
            std::this_thread::sleep_for(hearBeatC);
            CheckerHeader checkHeader;
            size_t buf_Pointer                          = 0;
            char chectHeaderBuff[CHECKER_HEADER_LENGTH] = { 0 };
            
            while (buf_Pointer < CHECKER_HEADER_LENGTH)
            {
                fcntl(socketID, F_SETFL,  O_NONBLOCK);
                if (read(socketID, &chectHeaderBuff[buf_Pointer], 1) <= 0)
                {
                    if (errno == EWOULDBLOCK){ break; }
                    else
                    {
                        std::cout<< "Disconnect To The Socket"<<std::endl;
                        close(socketID);
                        return;
                        //exit(12);
                    }
                }
                
                size_t error;
                
                if (buf_Pointer == CHECKER_HEADER_LENGTH-1)
                {
                    memcpy(&checkHeader, chectHeaderBuff, CHECKER_HEADER_LENGTH);
                    if(checkHeader.begin != 10001){ break; }
                    
                    //std::cout<<"Get Checker : "<<checkHeader.headerType<<std::endl;
                    switch (checkHeader.headerType) {
                        case HeaderType::MESSAGES_TYPE :
                        {
                            error = readMessage(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_TYPE :
                        {
                            error = readFile(socketID, checkedFileConformer);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_CONFORMER :
                        {
                            error = readFileConformer(socketID);
                            getError(error);
                            break;
                        }
   
                        default:
                            break;
                    }
                }
                buf_Pointer ++;
            }
        }
    }
    
    int YorkNetwork::readMessage(const int &socketID)
    {
        char MessageHeaderBuff[MESSAGE_HEADER_LENGTH]   = { 0 };
        int64_t buf_Context_Pointer                     = 0;
        MessageHeader messageHeader;
        char *messageContexBuff                         = nullptr;
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, &MessageHeaderBuff[0], MESSAGE_HEADER_LENGTH) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None MessageHeader Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read MessageHeader Data"<<std::endl;
                return errno;
            }
        }
        
        memcpy(&messageHeader, MessageHeaderBuff, MESSAGE_HEADER_LENGTH);
        if(messageHeader.begin!=1001 || messageHeader.length == -1)
        {
            return -1001;
        }
        
        messageContexBuff = new char[messageHeader.length];
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, &messageContexBuff[0], messageHeader.length) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None MessageContext Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read MessageContext Data"<<std::endl;
                return errno;
            }
        }
        
        didGetMessage(messageContexBuff);
        
        delete[] messageContexBuff;
        
        return 0;
    }
    
    int YorkNetwork::readFile(const int &socketID, int *indexOfCheckedBlock)
    {
        std::vector<RecivedData>fileContextList     = std::vector<RecivedData>();
        char headerBuff[HEADER_LENGTH]              = { 0 };
        char checkerBuff[CHECKER_HEADER_LENGTH]     = { 0 };
        char *contextBuff                           = new char[0];
        //bool headerChecked                          = false;
        bool fileDataBegin                          = false;
        int64_t lastBlockIndex                      = 0;
        std::string thisFileName                    = "";
        FileTypes   thisFileType                    = FileTypes::NONE;
        
        Header thisHeader;
        CheckerHeader checkHeader;
        
        std::string fileUniName                            = "";
        size_t buf_Pointer                                 = 0;
        
        while (1)
        {
            //std::this_thread::sleep_for(hearBeatC);
            if(fileDataBegin)
            {
                //std::this_thread::sleep_for(hearBeatC);
                buf_Pointer = 0;
                while (buf_Pointer < CHECKER_HEADER_LENGTH)
                {
                    std::this_thread::sleep_for(hearBeatC/3);
                    fcntl(socketID, F_SETFL,  O_NONBLOCK);
                    if (read(socketID, &checkerBuff[buf_Pointer], 1) <= 0)
                    {
                        if (errno == EWOULDBLOCK){ continue; }
                        else
                        {
                            std::cout<< "No message comes from server"<<std::endl;
                            //close(socketID);
                            return -1001;
                        }
                    }
                    //std::this_thread::sleep_for(hearBeatC/10);
                    if (buf_Pointer == CHECKER_HEADER_LENGTH-1)
                    {
                        memcpy(&checkHeader, checkerBuff, CHECKER_HEADER_LENGTH);
                        if(checkHeader.headerType != HeaderType::FILE_TYPE)
                        {
                            return -1001;
                        }
                    }
                    buf_Pointer++;
                }
            }
            
            fcntl(socketID, F_SETFL,  O_NONBLOCK);
            if (read(socketID, &headerBuff[0], HEADER_LENGTH) <= 0)
            {
                if (errno == EWOULDBLOCK)
                {
                    std::cout<< "None FileHeader Data Found"<<std::endl;
                    delete[] contextBuff;
                    return errno;
                    //break;
                }
                else
                {
                    std::cout<< "Error On Read FileHeader Data"<<std::endl;
                    delete[] contextBuff;
                    return errno;
                }
            }
            
            memcpy(&thisHeader, headerBuff, HEADER_LENGTH);
            if(thisHeader.begin!=10001 || thisHeader.length < 0 || thisHeader.totalBlock < thisHeader.indexOfBlock)
            {
                delete[] contextBuff;
                return -1001;
            }
            
            if(thisHeader.fileName != "")
            {
                if( !fileDataBegin )
                {
                    fileDataBegin   = true;
                    thisFileName    = thisHeader.fileName;
                    thisFileType    = thisHeader.fileType;
                    
                    fileUniName = std::to_string(socketID) + "_" + std::to_string(thisHeader.tag);
                    SentingFile thisRecivingOne = SentingFile(thisHeader.tag, thisHeader.indexOfBlock);
                    _recivingFiles.insert(std::pair<std::string, SentingFile>(fileUniName, thisRecivingOne));
                }
            }
            else
            {
                delete[] contextBuff;
                return -1001;
            }
            
            contextBuff = new char[thisHeader.length];
            
            fcntl(socketID, F_SETFL,  O_NONBLOCK);
            if (read(socketID, &contextBuff[0], thisHeader.length) <= 0)
            {
                if (errno == EWOULDBLOCK)
                {
                    std::cout<< "None File Data Found"<<std::endl;
                    delete[] contextBuff;
                    return errno;
                }
                else
                {
                    std::cout<< "Error On Read File Data"<<std::endl;
                    delete[] contextBuff;
                    return errno;;
                }
            }
            
            if(fileDataBegin && thisFileName == thisHeader.fileName && thisFileType == thisHeader.fileType)
            {
                if(lastBlockIndex > thisHeader.indexOfBlock)
                {
                    delete[] contextBuff;
                    return -1001;
                }
                
                lastBlockIndex = thisHeader.indexOfBlock;
                
                fileContextList.push_back(RecivedData(thisHeader,contextBuff));
                //std::cout <<  "===========Received "<< thisHeader.indexOfBlock<<":"<<thisHeader.totalBlock << std::endl;
                //std::cout << "content : "<< contextBuff<<std::endl;
                
                SentingFile thisRecivedOne = SentingFile(thisHeader.tag,thisHeader.indexOfBlock);
                char* fileConformer = createBuffer(thisRecivedOne);
                
                //std::this_thread::sleep_for(hearBeatC);
                fcntl(socketID, F_SETFL,  O_NONBLOCK);
                if (send(socketID, fileConformer, HEADER_LENGTH + SENTING_FILE_H_LENGTH, 0) < 0)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        std::cout<< "File Conformer Error1"<<std::endl;
                        delete[] contextBuff;
                        return errno;
                    }
                    else
                    {
                        std::cout<< "File Conformer Error2"<<std::endl;
                        delete[] contextBuff;
                        return errno;;
                    }
                }
                //std::cout <<  "===========Will Receive "<< thisHeader.indexOfBlock<<":"<<thisHeader.totalBlock<< std::endl;
                
                if(thisHeader.indexOfBlock == thisHeader.totalBlock)
                {
                    int64_t blockCount = thisHeader.totalBlock;
                    int64_t thisfileLength;
                    if(blockCount > 2)
                    {
                        thisfileLength = FILE_BUFFER_SIZE * ( blockCount -1 );
                        thisfileLength += fileContextList.at(blockCount-1).header.length;
                    }
                    else
                    {
                        thisfileLength = thisHeader.length;
                    }
                    char* fileDataTotal = new char[thisfileLength];
                    
                    int64_t filePostionPointer = 0;
                    LOOP(thisHeader.totalBlock)
                    {
                        //Index error
                        if(fileContextList.at(ii).header.indexOfBlock != (ii +1))
                        {
                            std::cout <<  "Recived file index error:" << std::endl;
                            delete[] contextBuff;
                            delete[] fileDataTotal;
                            return -1001;
                        }
                        
                        memcpy(fileDataTotal+filePostionPointer, fileContextList.at(ii).data, fileContextList.at(ii).header.length);
                        filePostionPointer += fileContextList.at(ii).header.length;
                        
                        delete[] fileContextList.at(ii).data;
                        
                        
                        //fileContextList.clear();
                    }
                    Header outHeader = Header(thisHeader.tag,thisfileLength,1,1,thisFileName,thisHeader.fileType);
                    didGetFileData(fileDataTotal, outHeader);
                    
                    _recivingFiles.erase(fileUniName);
                    
                    fileContextList.clear();
                    //delete[] contextBuff;
                    return 0;
                    
                }
            }

            
            
        }
        
        
        
        
        
        return 0;
    }
    
    int YorkNetwork::readFileConformer(const int &socketID)
    {
        char fileConfomerBuff[SENTING_FILE_H_LENGTH];
        SentingFile thisSentingFile;
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, fileConfomerBuff, SENTING_FILE_H_LENGTH) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None checkerHeader Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read checkerHeader Data"<<std::endl;
                return errno;
            }
        }
        
        memcpy(&thisSentingFile, fileConfomerBuff, SENTING_FILE_H_LENGTH);
        
        // to download next block
        //thisSentingFile.blockIndex ++;
        
        //std::cout<<"target has recived "<<thisSentingFile.blockIndex<<std::endl;
        
        std::string fileUniName = "local_file_" + std::to_string(thisSentingFile.fileID);
        if(_sentingFiles.find(fileUniName) != _sentingFiles.end())
        {
            _sentingFiles[fileUniName] = thisSentingFile;
        }
        else
        {
            if(_sentingFiles.size() != 0)
                std::cout<< "Error On Checking Senting List"<<std::endl;
        }
        //if(_sentingFiles)
        
        return 0;
    }
    
    //Fuction called when file did all recived
    void YorkNetwork::didGetFileData(const char *inMessage, const YorkNet::YorkNetwork::Header &header)
    {
        saveFile(inMessage, "temp", header.fileType, header.length);
        
        delete []inMessage;
        getFileListFromData(inMessage);
        didGetFile(header);
        
    }
    
    void YorkNetwork::didGetFile(const YorkNet::YorkNetwork::Header &header)
    {
        std::cout<<"Got File "<<header.fileName<< getStringByFileType(header.fileType) << std::endl;
    }
    
    void YorkNetwork::getError(size_t error)
    {
        switch (error) {
            case EBADF:
                std::cout << "An invalid descriptor was specified."  << std::endl;
                break;
            case ECONNRESET:
                std::cout << "Connection reset by peer."  << std::endl;
                break;
            case EDESTADDRREQ:
                std::cout << "The socket is not connection-mode, and no peer address is set."  << std::endl;
                break;
            case EFAULT:
                std::cout << "An invalid user space address was specified for an argument."  << std::endl;
                break;
            case EINTR:
                std::cout << "A signal occurred before any data was transmitted"  << std::endl;
                break;
            case EMSGSIZE:
                std::cout << "The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible."  << std::endl;
                break;
            case ENOBUFS:
                std::cout << "The output queue for a network interface was full. This generally indicates that the interface has stopped sending, but may be caused by transient congestion. (Normally, this does not occur in Linux. Packets are just silently dropped when a device queue overflows.)"  << std::endl;
                break;
            case ENOMEM:
                std::cout << "No memory available."  << std::endl;
                break;
            case EAGAIN:
            {
                std::cout << "The socket is marked nonblocking and the requested operation would block"  << std::endl;
                break;
            }
            case EACCES:
                std::cout << "Write permission is denied on the destination socket file, or search permission is denied for one of the directories the path prefix. "  << std::endl;
                break;
                
        }

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
    
    void YorkNetwork::saveFile(const char *inMessage, const std::string &name, const FileTypes &fileType, const int64_t &size)
    {
        std::string fileName = name + getStringByFileType(fileType);
        FILE *fileToWrite;
        if(fileType == FileTypes::TXT || fileType == FileTypes::JSON || fileType == FileTypes::NONE)
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "w+");
        }
        else
        {
            fileToWrite = fopen(getDirPath(fileName).c_str(), "wb");
        }
        fwrite(inMessage, sizeof(char), size, fileToWrite);
        fclose(fileToWrite);
        std::cout<<"Finsh on save file"<< std::endl;
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
    
    std::map<std::string, YorkNetwork::FileListOne> YorkNetwork::getFileListFromData(const char *ins)
    {
        std::map<std::string, FileListOne> out = std::map<std::string, FileListOne>();
        size_t size = strlen(ins);
        std::string name = "";
        std::string type = "";
        int version = 0;
        int caseFlag = 0;
        char tempContainer[20];
        int pointer = 0;
        bzero(tempContainer, 20);
        LOOP(size)
        {
            //tempContainer[pointer] = ins[ii];
            //name
            if(caseFlag == 0)
            {
                if(ins[ii] == ' ')
                {
                    name = tempContainer;
                    pointer = 0;
                    caseFlag ++;
                    bzero(tempContainer, 20);
                    continue;
                }
                else
                {
                    tempContainer[pointer] = ins[ii];
                }
            }
            //type
            else if (caseFlag == 1)
            {
                if(ins[ii] == ' ')
                {
                    type = tempContainer;
                    pointer = 0;
                    caseFlag ++;
                    bzero(tempContainer, 20);
                    continue;
                }
                else
                {
                    tempContainer[pointer] = ins[ii];
                }
            }
            //version
            else if (caseFlag == 2)
            {
                if(ins[ii] == '\n' || ii == size-1)
                {
                    version = atoi(tempContainer);
                    pointer = 0;
                    FileListOne oo = FileListOne(name,getFileType(type),version);
                    out.insert(std::pair<std::string, FileListOne>(name, oo));
                    caseFlag = 0;
                    bzero(tempContainer, 20);
                    continue;
                }
                else
                {
                    tempContainer[pointer] = ins[ii];
                }
            }
            
            pointer++;
            
        }
        
        return out;
    }
    
    void YorkNetwork::getFileListFormFile()
    {
        std::string fileAbslutPath = getDirPath("fileList.txt");
        // count fileBlock sent
        int64_t fileSize = getFileSize(fileAbslutPath);
        char* fileListData = new char[fileSize];
        
        FILE *fileR;
        
        fileR = std::fopen(fileAbslutPath.c_str(), "r");
        if(fileR == NULL)
        {
            fclose(fileR);
            std::cout << "Can not Open File "<< fileAbslutPath << std::endl;
            return;
        }
        int64_t file_block_length = fread(fileListData, sizeof(char), FILE_BUFFER_SIZE, fileR);
        
        if(file_block_length != fileSize)
        {
            fclose(fileR);
            std::cout << "Can not Read File "<< fileAbslutPath << std::endl;
        }
        
        _fileList = getFileListFromData(fileListData);
        
    }
    
    
    const char* YorkNetwork::getDataFromFileList(std::map<std::string, FileListOne> ins)
    {
        std::string toSave = "";
        std::map<std::string, YorkNetwork::FileListOne>::iterator itor;
        for (itor = ins.begin(); itor != ins.end(); itor++)
        {
            FileListOne one = itor->second;
            toSave += one.name;
            toSave += " ";
            toSave += one.type;
            toSave += " ";
            toSave += std::to_string(one.version);
            toSave += "\r";
        }
        
        const char *out = toSave.c_str();
        
        return out;
    }
    
    
    

} /* namespace York */
