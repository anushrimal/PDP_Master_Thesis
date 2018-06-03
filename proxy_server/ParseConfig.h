#ifndef _PARSE_CONFIG_H_
#define _PARSE_CONFIG_H_

#include<iostream>
#include<string>
#include<map>

using namespace std;

class ParseConfig
{
public:
	ParseConfig():mPortNum(-1) 
	{}

	bool parse(string cfg_file);
	int mPortNum;
	string mMode;
	string mPath;
	multimap<string, unsigned int> mCNs;	
private:
	static constexpr unsigned int hash(const char* str, int h = 0)	
	{
    		return !str[h] ? 5381 : (hash(str, h+1)*33) ^ str[h];
	}
};

#endif
