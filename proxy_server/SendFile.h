#ifndef _SEND_FILE_H_
#define _SEND_FILE_H_

#include<string>

using namespace std;

class SendFile
{
protected:
	string mNodeIP;
	unsigned int mPortNum;

	string exec(const char *cmd);
	 
public:
	SendFile(string nodeIP, unsigned int portNum);
	bool sendFile(char *fileName, char *fileData);
	string getFile(const char *fileName);	
	string challenge(const char* fileName, string params);
};

#endif
