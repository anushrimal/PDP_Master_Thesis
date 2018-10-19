#ifndef _RPI_AUTH_H_
#define _RPI_AUTH_H_

#include <string>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/call_traits.hpp>
/*#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>*/
#include <boost/next_prior.hpp>
#include <boost/tokenizer.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include "DBHandler.h"
#include "Utils.h"

namespace pt = boost::property_tree;
/*using namespace boost::multi_index;

struct lev_blocknum_key:composite_key<
  auth_table,
  BOOST_MULTI_INDEX_MEMBER(auth_table, int, level),
  BOOST_MULTI_INDEX_MEMBER(auth_table, int, blocknum)
>{};

struct lev_rpi_key:composite_key<
  auth_table,
  BOOST_MULTI_INDEX_MEMBER(auth_table, int, level),
  BOOST_MULTI_INDEX_MEMBER(auth_table, int, rpi)
>{};

typedef multi_index_container<
	auth_table,
	indexed_by<
		ordered_non_unique<
			lev_blocknum_key
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
			,composite_key_result_less<lev_blocknum_key::result_type>
#endif
		>,
		ordered_non_unique<
			lev_rpi_key
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
			,composite_key_result_less<lev_rpi_key::result_type>
#endif
        	>
	> 
> auth_set;

* typedef's of the two indices of auth_table *

typedef nth_index<auth_set,0>::type auth_entry_by_level_n_blocknum;
typedef nth_index<auth_set,1>::type auth_entry_by_level_n_rpi;*/

class RpiAuth
{
private:
	void sortByLevel();
	int int_element_at(pt::ptree const& pt, std::string name, size_t n); 
	string string_element_at(pt::ptree const& pt, std::string name, size_t n); 
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
