#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <memory>
#include <stdexcept>
#include <array>
#include "SendFile.h"

using namespace std;

SendFile::SendFile(string nodeIP, unsigned int portNum)
{
	mNodeIP = nodeIP;
	mPortNum = portNum;		
}

bool SendFile::sendFile(char *fileName, char *fileData)
{
	cout<<"Size of fileData : "<<strlen(fileData)<<endl;
	// curl -w'\n' -v --header "Transfer-Encoding: chunked" -F "FileName=first.txt" -F "FileData=@./resources/sample.txt" 'http://localhost:7777/resources/'
	string curlCmd =  "curl -w'\n' -v --header \"Transfer-Encoding: chunked\" -F \"FileName=";
	curlCmd += fileName;
	curlCmd += "\" -F \"FileData=@";
	curlCmd += fileData;
	curlCmd += "\" 'http://";
	curlCmd += mNodeIP.c_str();
	curlCmd += ":";
	curlCmd += to_string(mPortNum);
	curlCmd += "/resources'";
	//cout<<curlCmd<<endl;
	int ret = system(curlCmd.c_str());
	cout<<"Send file status :"<<ret;
	if(ret >= 0)
		return 0;
}

string SendFile::getFile(const char *fileName)
{
	string curlCmd =  "curl -w'\n' -v -XGET 'http://";
	curlCmd += mNodeIP;
	curlCmd += ":";
	curlCmd += to_string(mPortNum);
	curlCmd += "/resources/";
	curlCmd += fileName;
	curlCmd += "'";
	cout<<curlCmd<<endl;
	string ret = exec(curlCmd.c_str());
	//cout<<" file status :"<<ret;
	return ret;
}

string SendFile::exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}	
