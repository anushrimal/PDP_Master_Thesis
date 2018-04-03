#include <iostream>
#include <stdlib.h>
#include <string>
#include "SendFile.h"

using namespace std;

SendFile::SendFile(char *nodeIP, int portNum)
{
	mNodeIP = nodeIP;
	mPortNum = portNum;		
}

bool SendFile::sendFile(char *fileName, char *fileData)
{
	// curl -w'\n' -v --header "Transfer-Encoding: chunked" -F "FileName=first.txt" -F "FileData=@./resources/sample.txt" 'http://localhost:7777/resources/'
	string curlCmd =  "curl -w'\n' -v --header \"Transfer-Encoding: chunked\" -F \"FileName=";
	curlCmd += fileName;
	curlCmd += "\" -F \"FileData=";
	curlCmd += fileData;
	curlCmd += "\" 'http://";
	curlCmd += mNodeIP;
	curlCmd += ":";
	curlCmd += to_string(mPortNum);
	curlCmd += "/resources'";
	cout<<curlCmd<<endl;
	int ret = system(curlCmd.c_str());
	cout<<"Send file status :"<<ret;
	if(ret >= 0)
		return 0;
}
