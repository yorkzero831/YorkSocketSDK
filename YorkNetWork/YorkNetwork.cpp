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
    
    char* YorkNetwork::createBufferForFile( char *preBuffer,  int64_t tag, int64_t numOfBlock,  int64_t indexOfBlock,  std::string fileName,  FileTypes fileType, int64_t fileLength)
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
    
    char* YorkNetwork::createBufferForMessage(const std::string &message)
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
    
    char* YorkNetwork::createBufferForConformer(const SentingFile &thisSentingFile)
    {
        char *out = new char[CHECKER_HEADER_LENGTH + SENTING_FILE_H_LENGTH];
        CheckerHeader cekHeader = CheckerHeader(HeaderType::FILE_CONFORMER);
        
        memcpy(out, &cekHeader , CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &thisSentingFile, SENTING_FILE_H_LENGTH);
        
        return out;
    }
    
    char* YorkNetwork::createBufferForFileList(const YorkNet::YorkNetwork::FileListHeader &thisFRHeader, const char* fileRequestListData, const HeaderType &type)
    {
        int64_t dataSize = strlen(fileRequestListData);
        
        char *out = new char[CHECKER_HEADER_LENGTH + FILES_LIST_LENGTH + dataSize];
        CheckerHeader cekHeader;
       
        cekHeader = CheckerHeader(type);
        
        
        memcpy(out, &cekHeader , CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &thisFRHeader, FILES_LIST_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH + FILES_LIST_LENGTH, fileRequestListData, dataSize);
        
        delete [] fileRequestListData;
        
        return out;
    }
    
    char* YorkNetwork::createBufferForCommand(const YorkNet::YorkNetwork::CommandTypes &cmd)
    {
        char* out = new char[CHECKER_HEADER_LENGTH + COMMAND_HEADER_LENGTH];
        CheckerHeader cekHeader = CheckerHeader(HeaderType::COMMAND_TYPE);
        CommandHeader cmdHeader = CommandHeader(cmd);
        
        memcpy(out, &cekHeader , CHECKER_HEADER_LENGTH);
        
        memcpy(out + CHECKER_HEADER_LENGTH, &cmdHeader, COMMAND_HEADER_LENGTH);
        
        return out;
    }
    
    //Function called when sent file to socket
    int YorkNetwork::sentFileToSocket(const int &socketID, std::string fileName, std::string fileType, const YorkNet::YorkNetwork::SentingFile &fileConformerH)
    {
        std::string fileUniName ;
        
        int64_t fileBlockPos;
        int64_t fileBlockTotal;
        int64_t fileSize;
        FileTypes fileTypeT;
        int64_t thisBlockNum;
        int64_t fid;
        
        if(fileConformerH.fileID == -1)
        {
            _fileID++;
            fid = _fileID;
            //put this file into sentingFile list
            fileUniName = "local_file_" + std::to_string(_fileID);
        }
        else
        {
            fid = fileConformerH.fileID;
            fileUniName = "local_file_" + std::to_string(fid);
        }
        
        FILE *fileR;
        
        if(_openingFiles.find(fileUniName)== _openingFiles.end())
        {
            std::string filePath       = fileName + "." + fileType;
            std::string fileAbslutPath = getDirPath(filePath);
            
            // count fileBlock sent
            fileSize = getFileSize(fileAbslutPath);

            fileBlockTotal = fileSize/FILE_BUFFER_SIZE;
            if(fileSize%FILE_BUFFER_SIZE > 0)
                fileBlockTotal++;
            
            fileTypeT = getFileType(fileType);
            
            
            
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
            
            _openingFiles.insert(std::pair<std::string, OpeningFile>(fileUniName, OpeningFile(fileR, fileName, fileBlockTotal, fileSize, fileTypeT)));
            
            _sentingFiles.insert(std::pair<std::string, SentingFile>(fileUniName, SentingFile(fid,0)));
            fileBlockPos = 0;
            thisBlockNum = 0;
        }
        else
        {
            fileR           = _openingFiles[fileUniName].file;
            fileSize        = _openingFiles[fileUniName].fileSize;
            fileBlockTotal  = _openingFiles[fileUniName].indexCount;
            fileTypeT       = _openingFiles[fileUniName].filetype;
            thisBlockNum    = _sentingFiles[fileUniName].blockIndex;
            fileBlockPos    = thisBlockNum * FILE_BUFFER_SIZE;
            
            
            fseek(fileR, fileBlockPos, SEEK_SET);
        }
        
        char buffer[FILE_BUFFER_SIZE];
        
        //int64_t thisBlockNum = _sentingFiles[fileUniName].blockIndex ;
        
        bzero(buffer, FILE_BUFFER_SIZE);
        int file_block_length = 0;
       
        if( (file_block_length = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, fileR)) > 0)
        {
            
            //std::this_thread::sleep_for(heartBeatC);
#ifdef FILE_SENT_DEBUG
            std::cout << "file_block_length: " << file_block_length << std::endl;
            std::cout << "content: " << buffer << std::endl;
#endif
            thisBlockNum ++;
            
           
            int64_t blockLenth;
            if(thisBlockNum < fileBlockTotal){ blockLenth = FILE_BUFFER_SIZE;}
            else if (thisBlockNum == fileBlockTotal) { blockLenth = fileSize%FILE_BUFFER_SIZE ;}
            
            char *sentChar = createBufferForFile(buffer, fid, fileBlockTotal, thisBlockNum, fileName, fileTypeT, blockLenth);
            int64_t sentLenth = strlen(buffer);
            
            fcntl(socketID, F_SETFL,  O_NONBLOCK);
            if(send(socketID, sentChar, blockLenth + HEADER_LENGTH + CHECKER_HEADER_LENGTH, 0) < 0)
            {
                getError(errno);
                
                delete[] sentChar;
                fclose(fileR);
                _sentingFiles.erase(fileUniName);
                _openingFiles.erase(fileUniName);
                
                std::cout << "Error on sending file"  << std::endl;
                return -1001;
            }
            //std::cout <<  "===========Send "<< thisBlockNum<<":"<< fileBlockTotal<< std::endl;
            
            delete[] sentChar;
            
            bzero(buffer, FILE_BUFFER_SIZE);
            
        }
        
        if(thisBlockNum == fileBlockTotal)
        {
            fclose(fileR);
            _sentingFiles.erase(fileUniName);
            _openingFiles.erase(fileUniName);
            std::cout<<"Sent File"<<fileName<<"."<<fileType<<std::endl;
        }

        
        return 0;
    }
    
    int YorkNetwork::sentFileListToSocket(const int &socketID, const char *fileRequestData, const int64_t &fileCount)
    {
        int64_t dataSize = strlen(fileRequestData);
        
        FileListHeader thisFRHeader = FileListHeader(dataSize, fileCount);
        
        char *sentBuff = createBufferForFileList(thisFRHeader, fileRequestData, HeaderType::FILE_LIST);
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if(send(socketID, sentBuff, dataSize + FILES_LIST_LENGTH + CHECKER_HEADER_LENGTH, 0) < 0)
        {
            getError(errno);
            
            delete[] sentBuff;
            std::cout << "Error on sending file"  << std::endl;
            return -1001;
        }
        
        delete[] sentBuff;
        return 0;
    }
    
    int YorkNetwork::sentFileRequestToSocket(const int &socketID, const char *fileRequestData, const int64_t &fileCount, const HeaderType &type)
    {
        FileListHeader thisFRHeader = FileListHeader(strlen(fileRequestData), fileCount);
        char *sentBuff = createBufferForFileList(thisFRHeader, fileRequestData, type);
    
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if(write(socketID, sentBuff, thisFRHeader.dataLength + FILES_LIST_LENGTH + CHECKER_HEADER_LENGTH) < 0)
        {
            getError(errno);
            
            delete[] sentBuff;
            std::cout << "Error on sending file"  << std::endl;
            return -1001;
        }
        //std::this_thread::sleep_for(heartBeatC);
        delete [] sentBuff;
        return 0;
    }
    
    int YorkNetwork::sentCommandToSocket(const int &socketID, const YorkNet::YorkNetwork::CommandTypes &cmd)
    {
        char *cmdBuf = createBufferForCommand(cmd);
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if(send(socketID, cmdBuf, COMMAND_HEADER_LENGTH + CHECKER_HEADER_LENGTH, 0) < 0)
        {
            getError(errno);
            
            std::cout << "Error on sending file"  << std::endl;
            return -1001;
        }
        
        delete cmdBuf;
        return 0;
    }
    
    void YorkNetwork::readFromSocket(const int &socketID)
    {
        int *checkedFileConformer                   = 0;
        
        while (1)
        {
            if(hostType == HostType::CLIENT)
                std::this_thread::sleep_for(heartBeatC);
            if(hostType == HostType::SERVER)
                std::this_thread::sleep_for(heartBeatC);
            
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
                
                
                
                if (buf_Pointer == CHECKER_HEADER_LENGTH-1)
                {
                    int64_t error = 0;
                    
                    memcpy(&checkHeader, chectHeaderBuff, CHECKER_HEADER_LENGTH);
                    if(checkHeader.begin != 10001){ break; }
                    
                    //std::cout<<"Get Checker : "<<checkHeader.headerType<<std::endl;
                    switch (checkHeader.headerType)
                    {
                        case HeaderType::MESSAGES_TYPE :
                        {
                            error = readMessageFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_TYPE :
                        {
                            error = readFileFromSocket2(socketID, checkedFileConformer);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_CONFORMER :
                        {
                            error = readFileConformerFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_LIST :
                        {
                            error = readFileListFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::COMMAND_TYPE :
                        {
                            error = readCommandFromSocket(socketID);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_REQUEST_NEED_TO_SEND :
                        {
                            error = readFileRequestFromSocket(socketID, FILE_REQUEST_NEED_TO_SEND);
                            getError(error);
                            break;
                        }
                        case HeaderType::FILE_REQUEST_NEED_TO_RECIEVE :
                        {
                            error = readFileRequestFromSocket(socketID, FILE_REQUEST_NEED_TO_RECIEVE);
                            getError(error);
                            break;
                        }
                        
                        
   
                        default:
                            break;
                    }
                    if(error != 0)
                    {
                        close(socketID);
                        //close(socketID);
                        std::cout<<"========= Some ERROR Happened"<<std::endl;
                    }
                }
                buf_Pointer ++;
            }
        }
    }
    
    int YorkNetwork::readMessageFromSocket(const int &socketID)
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
        if(messageHeader.begin!=10001 || messageHeader.length == -1)
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
    
    int YorkNetwork::readFileFromSocket2(const int &socketID, int *indexOfCheckedBlock)
    {
        std::vector<RecivedData>*fileContextList;
        char headerBuff[HEADER_LENGTH]              = { 0 };
        char checkerBuff[CHECKER_HEADER_LENGTH]     = { 0 };
        char *contextBuff                           = nullptr;
        int64_t lastBlockIndex                      = 0;
        std::string thisFileName                    = "";
        FileTypes   thisFileType                    = FileTypes::NONE;
        
        Header thisHeader;
        CheckerHeader checkHeader;
        
        std::string fileUniName                            = "";
        size_t buf_Pointer                                 = 0;
        
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
            thisFileName    = thisHeader.fileName;
            thisFileType    = thisHeader.fileType;
            
            fileUniName = std::to_string(socketID) + "_" + std::to_string(thisHeader.tag);
            if(_recivingFiles.find(fileUniName) == _recivingFiles.end())
            {
                SentingFile thisRecivingOne = SentingFile(thisHeader.tag, thisHeader.indexOfBlock);
                _recivingFiles.insert(std::pair<std::string, SentingFile>(fileUniName, thisRecivingOne));
                
                fileContextList = new std::vector<RecivedData>();
                _recivingDatas.insert(std::pair<std::string, std::vector<RecivedData>*>(fileUniName, fileContextList));
            }
            else
            {
                fileContextList = _recivingDatas[fileUniName];
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
        
        if(thisFileName == thisHeader.fileName && thisFileType == thisHeader.fileType)
        {
//            if(lastBlockIndex > thisHeader.indexOfBlock)
//            {
//                delete[] contextBuff;
//                return -1001;
//            }
            
            lastBlockIndex = thisHeader.indexOfBlock;
            fileContextList->push_back(RecivedData(thisHeader,contextBuff));
#ifdef FILE_RECIEVE_DEBUG
            std::cout <<  "===========Received "<< thisHeader.indexOfBlock<<":"<<thisHeader.totalBlock << std::endl;
            std::cout << "content : "<< contextBuff<<std::endl;
#endif
            if(thisHeader.indexOfBlock < thisHeader.totalBlock)
            {
                SentingFile thisRecivedOne = SentingFile(thisHeader.tag,thisHeader.indexOfBlock);
                char* fileConformer = createBufferForConformer(thisRecivedOne);
                
                fcntl(socketID, F_SETFL,  O_NONBLOCK);
                if (send(socketID, fileConformer, CHECKER_HEADER_LENGTH + SENTING_FILE_H_LENGTH, 0) < 0)
                {
                    size_t coutO = fileContextList->size();
                    LOOP(coutO)
                    {
                        delete [] fileContextList->at(ii).data;
                    }
                    delete fileContextList;
                    
                    if (errno == EWOULDBLOCK)
                    {
                        std::cout<< "File Conformer Error1"<<std::endl;
                        delete[] fileConformer;
                        delete[] contextBuff;
                        return errno;
                    }
                    else
                    {
                        std::cout<< "File Conformer Error2"<<std::endl;
                        delete[] fileConformer;
                        //delete[] contextBuff;
                        return errno;;
                    }
                }
                
                delete[] fileConformer;
                fileConformer = NULL;
                //std::cout <<  "===========Will Receive "<< thisHeader.indexOfBlock<<":"<<thisHeader.totalBlock<< std::endl;
            }
            
            
            
        }

        if(thisHeader.indexOfBlock == thisHeader.totalBlock)
        {
            int64_t blockCount = thisHeader.totalBlock;
            int64_t thisfileLength;
            if(blockCount > 1)
            {
                thisfileLength = FILE_BUFFER_SIZE * ( blockCount -1 );
                thisfileLength += fileContextList->at(blockCount-1).header.length;
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
                if(fileContextList->at(ii).header.indexOfBlock != (ii +1))
                {
                    std::cout <<  "Recived file index error:" << std::endl;
                    delete[] contextBuff;
                    delete[] fileDataTotal;
                    delete fileContextList;
                    for (long i = ii; i < thisHeader.totalBlock; i++)
                    {
                        //Problem;
                        //delete [] fileContextList->at(i).data;
                    }
                    return -1001;
                }
                
                memcpy(fileDataTotal+filePostionPointer, fileContextList->at(ii).data, fileContextList->at(ii).header.length);
                filePostionPointer += fileContextList->at(ii).header.length;
                
                delete[] fileContextList->at(ii).data;
                fileContextList->at(ii).data = NULL;
            }
            //delete[] contextBuff;
            
            Header outHeader = Header(thisHeader.tag,thisfileLength,1,1,thisFileName,thisHeader.fileType);
            //std::cout <<  "Recived file data "<<thisHeader.fileName << std::endl;
            didGetFileData(fileDataTotal, outHeader);
            
            _recivingFiles.erase(fileUniName);
            
            fileContextList->clear();
            delete fileContextList;
            
            _recivingDatas.erase(fileUniName);
            //delete[] contextBuff;
            return 0;
            
        }

        
        return 0;
    }
    
    int YorkNetwork::readFileConformerFromSocket(const int &socketID)
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
        if(_openingFiles.find(fileUniName)!= _openingFiles.end())
        {
            OpeningFile thisOpeningFile = _openingFiles[fileUniName];
            sentFileToSocket(socketID, thisOpeningFile.fileName, getStringByFileType(thisOpeningFile.filetype) , thisSentingFile);
            
        }
        
        
        
        return 0;
    }
    
    int YorkNetwork::readFileListFromSocket(const int &socketID)
    {
        //std::this_thread::sleep_for(heartBeatC);
        char fileListHeaderBuff[FILES_LIST_LENGTH];
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, fileListHeaderBuff, FILES_LIST_LENGTH) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None FileListHeader Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read FileListHeader Data"<<std::endl;
                return errno;
            }
        }
        
        FileListHeader thisRequest;
        memcpy(&thisRequest, fileListHeaderBuff, FILES_LIST_LENGTH);
        
        if(thisRequest.begin != 10001)
        {
            return -1001;
        }
        
        std::this_thread::sleep_for(heartBeatC);
        char *requestData = new char[thisRequest.dataLength];
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, requestData, thisRequest.dataLength) <= 0)
        {
            delete[] requestData;
            
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None FileList Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read FileList Data"<<std::endl;
                return errno;
            }
        }
        std::cout<< "Do Get File List Process"<<std::endl;
        
        std::map<std::string, YorkNetwork::FileListOne> recievedFileList = getFileListFromData(requestData);
        didGetFileList(recievedFileList, socketID);
        
        //delete[] requestData;
        return 0;
    }
    
    int YorkNetwork::readFileRequestFromSocket(const int &socketID, const HeaderType &type)
    {
        char fileRequestHeaderBuff[FILES_LIST_LENGTH];
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, fileRequestHeaderBuff, FILES_LIST_LENGTH) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None FileListHeader Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read FileListHeader Data"<<std::endl;
                return errno;
            }
        }
        
        FileListHeader thisRequest;
        memcpy(&thisRequest, fileRequestHeaderBuff, FILES_LIST_LENGTH);
        
        if(thisRequest.begin != 10001)
        {
            std::cout<< "File Request Header READ ERROR"<<std::endl;
            return -1001;
        }
        
        if(thisRequest.count == 0)
        {
            std::map<std::string, FileListOne> getFileList;
            didGetFileRequestList(getFileList, socketID, type);
            return 0;
        }
        char *requestData = new char[thisRequest.dataLength];
        
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, requestData, thisRequest.dataLength) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "None FileList Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read FileList Data"<<std::endl;
                return errno;
            }
        }
        
        std::map<std::string, FileListOne> getFileList = getFileListFromData(requestData);
        
        if (getFileList.size() != thisRequest.count)
        {
            return -1001;
        }
        
        didGetFileRequestList(getFileList, socketID, type);
        return 0;
    }
    
    int YorkNetwork::readCommandFromSocket(const int &socketID)
    {
        char cmdHeaderBuff[COMMAND_HEADER_LENGTH];
        fcntl(socketID, F_SETFL,  O_NONBLOCK);
        if (read(socketID, &cmdHeaderBuff[0], COMMAND_HEADER_LENGTH) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout<< "Command Data Found"<<std::endl;
                return errno;
                //break;
            }
            else
            {
                std::cout<< "Error On Read Command Data"<<std::endl;
                return errno;
            }
        }
        
        CommandHeader cmdHeader;
        
        memcpy(&cmdHeader, cmdHeaderBuff, MESSAGE_HEADER_LENGTH);
        if(cmdHeader.begin != 10001)
        {
            return -1001;
        }
        
        didGetCommand(cmdHeader, socketID);
        
        return 0;
    }
    
    //Fuction called when file did all recived
    void YorkNetwork::didGetFileData(const char *inMessage, const YorkNet::YorkNetwork::Header &header)
    {
        std::cout<< "Success Saved "<< header.fileName << "."<< getStringByFileType(header.fileType) <<std::endl;
        saveFile(inMessage, header.fileName, header.fileType, header.length);
        
        delete []inMessage;
        //getFileListFromData(inMessage);
        didGetFile(header);
        
    }
    
    void YorkNetwork::didGetFile(const YorkNet::YorkNetwork::Header &header)
    {
        //std::cout<<"Got File "<<header.fileName<< getStringByFileType(header.fileType) << std::endl;
    }
    
    void YorkNetwork::getError(size_t error)
    {
        //std::cout<<"===========";
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
        std::string fileName = name + "." +getStringByFileType(fileType);
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
        //std::cout<<"Finsh on save file"<< std::endl;
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
                out = "png";
                break;
            case FileTypes::JSON :
                out = "json";
                break;
            case FileTypes::JPG :
                out = "jpg";
                break;
            case FileTypes::TXT :
                out = "txt";
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
        //out.clear();
        
        size_t size = strlen(ins);
        std::string name = "";
        std::string type = "";
        std::string versionS = "";
        int caseFlag = 0;
        char tempContainer[20] ={ 0 };
        int pointer = 0;
        bzero(tempContainer, 20);
    
        LOOP(size)
        {
            
            if(ins[ii] == '\n' || ins[ii] == '\r' || ii == pointer-1)
            {
                
                for (int i = 0; i < pointer; i++)
                {
                    if(caseFlag == 0)
                    {
                        if(tempContainer[i] == ' ')
                        {
                            caseFlag ++;
                        }
                        else
                        {
                            name += tempContainer[i];
                        }
                    }
                    else if(caseFlag == 1)
                    {
                        if(tempContainer[i] == ' ')
                        {
                            caseFlag ++;
                        }
                        else
                        {
                            type += tempContainer[i];
                        }
                        
                    }
                    else if(caseFlag == 2)
                    {
                        versionS += tempContainer[i];
                        if(i == pointer -1)
                        {
                            FileListOne oo = FileListOne(name,getFileType(type),std::atoi(versionS.c_str()));
                            out.insert(std::pair<std::string, FileListOne>(name, oo));
                            
                            name        = "";
                            type        = "";
                            versionS    = "";
                            caseFlag    = 0;
                        }

                    }
                    
                }
                
                bzero(tempContainer, pointer+1);
                pointer = -1;
            }
            else
            {
                tempContainer[pointer] = ins[ii];
            }
            pointer++;
        }
        
        std::cout << "File List count "<< out.size() << std::endl;
        delete [] ins;
        return out;
    }
    
    const char* YorkNetwork::getFileListDataFormFile()
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
            return nullptr;
        }
        int64_t file_block_length = fread(fileListData, sizeof(char), FILE_BUFFER_SIZE, fileR);
        fclose(fileR);
        if(file_block_length != fileSize)
        {
            //fclose(fileR);
            std::cout << "Can not Read File "<< fileAbslutPath << std::endl;
        }
        
        return fileListData;
        //_fileList = getFileListFromData(fileListData);
        
    }
    
    const char* YorkNetwork::getDataFromFileList(std::map<std::string, FileListOne> ins)
    {
        std::string toSave = "";
        std::map<std::string, YorkNetwork::FileListOne>::iterator itor;
        for (itor = ins.begin(); itor != ins.end(); itor++)
        {
            FileListOne one = itor->second;
            toSave.append(one.name);
            toSave.append(" ");
            toSave.append(getStringByFileType(one.type));
            toSave.append(" ");
            toSave.append(std::to_string(one.version));
            toSave.append("\r");
        }
        char *out = new char[toSave.size()];
        strcpy(out, toSave.c_str());
        
        return out;
    }
    
    const char* YorkNetwork::getDataFromFileList(std::vector<FileListOne> ins)
    {
        std::string toSave = "";
        LOOP(ins.size())
        {
            FileListOne one = ins.at(ii);
            toSave.append(one.name);
            toSave.append(" ");
            toSave.append(getStringByFileType(one.type));
            toSave.append(" ");
            toSave.append(std::to_string(one.version));
            toSave.append("\r");
        }
        char *out = new char[toSave.size()];
        strcpy(out, toSave.c_str());
        
        return out;
    }
    
    
    

} /* namespace York */
