#ifndef _SEND_FILE_H_
#define _SEND_FILE_H_

class SendFile
{
protected:
	char *mNodeIP;
	int mPortNum;
	 
public:
	SendFile(char *nodeIP, int portNum);
	bool sendFile(char *fileName, char *fileData);
	
};

#endif
