#ifndef _RPI_AUTH_H_
#define _RPI_AUTH_H_

#include <string>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/call_traits.hpp>
#include <boost/next_prior.hpp>
#include <boost/tokenizer.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include "DBHandler.h"
#include "Utils.h"

namespace pt = boost::property_tree;

class RpiAuth
{
private:
	void sortByLevel();
	int int_element_at(pt::ptree const& pt, std::string name, size_t n); 
	string string_element_at(pt::ptree const& pt, std::string name, size_t n); 
	bool hasSiblings(int cur_rpi, int test_rpi, int level, vector<int>& rpi_list);

public:
	int order;
	int levels;
	int blocksize;
	int blockcount;
	multimap<int, auth_table*> fileauth;	
	vector<int> level;
	vector<int> blocknum;
	vector<int> rpi;
	vector<string> hash;
	
	bool parse(DBHandler *db, const char* filename);
	bool validate(vector<int> b, vector<string> h, string& topHash);
};
#endif
