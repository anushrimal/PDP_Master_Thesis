#ifndef _SEND_FILE_H_
#define _SEND_FILE_H_

#include<string>
class SendFile
{
protected:
	char *mNodeIP;
	int mPortNum;

	std::string exec(const char *cmd);
	 
public:
	SendFile(char *nodeIP, int portNum);
	bool sendFile(char *fileName, char *fileData);
	std::string getFile(const char *fileName);	
};

#endif
