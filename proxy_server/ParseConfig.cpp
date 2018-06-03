#include "ParseConfig.h"
#include <fstream>

bool ParseConfig::parse(string cfg_file)
{
	std::ifstream infile(cfg_file);
	for(string line; getline(infile, line); )
	{
		if(line.empty() || line.length() < 6 || line[0] == '#') {
			continue;
		} 
		int eqPos = line.find("=");
		if(eqPos == string::npos) {
			continue;
		}
		string name = line.substr(0, eqPos);
		string val = line.substr(eqPos + 1, line.length());
		switch (hash(name.c_str()))
		{
			case hash("port"):
            			cout << "Port No : " <<val<<endl;
				mPortNum = stol(val);
				if(mPortNum  < 128 || mPortNum > 65535) {
					cout<<"Invalid Port..Exiting\n";
					return false;
				}
            			break;
        		case hash("mode"):
				cout << "Mode : "<<val<<endl;
				mMode = val;
            			break;
			case hash("path"):
				cout << "Mode : "<<val<<endl;
				mPath = val;
				break;
			case hash("cn"):
				int col = val.find(":");
				if(col == string::npos) {
					cout<<"Invalid entry : "<<line<<endl;
					return false;
				}
				string ip = val.substr(0, col);
				unsigned int port = stoul(val.substr(col + 1, val.length()));
				cout << "\tIP : "<<ip<< " Port : "<<port<<endl;
				mCNs.insert(std::pair<string, unsigned int>(ip, port));
				break;
        		default:
            			break;
		}
	}
	
	if(mPortNum == -1 || (!(mMode == "ps" || mMode == "cn")))
		return false;
	else 
		return true; 		
}

