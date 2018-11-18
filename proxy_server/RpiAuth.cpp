#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <iterator>
#include <sstream>
#include "RpiAuth.h"
#include "Utils.h"

using namespace std;

int RpiAuth::int_element_at(pt::ptree const& pt, string name, size_t n) {
	return std::next(pt.get_child(name).find(""), n)->second.get_value<int>();
}

string RpiAuth::string_element_at(pt::ptree const& pt, string name, size_t n) {
	return std::next(pt.get_child(name).find(""), n)->second.get_value<string>();
}

bool RpiAuth::parse(DBHandler *db, const char* filename)
{
	if(db == NULL) {
		cout<<"Error : File not found\n";
		return NULL;
	}
	string ret = db->readFile(filename);
	std::stringstream ss;
	ss << ret;
	cout<<ret<<endl;
	pt::ptree proot;
	pt::read_json(ss, proot);
	this->levels = proot.get<int>("levels");
	this->order = proot.get<int>("order");
	this->blockcount = proot.get<int>("blockcount");
	this->blocksize = proot.get<int>("blocksize");

	vector<int> r;
	vector<int> b;
	vector<string> h;
	int cur_lev= -1, prev_lev = -1;
	BOOST_FOREACH(pt::ptree::value_type& v, proot) { // iterate over immediate children of the root
		int i = v.first.find("_");
		if(i > 3) { // Level specific info
			cur_lev = stoi(v.first.substr(3, i));
			if(cur_lev != prev_lev) {
				for(int k = 0; k < r.size(); k++) {
					level.push_back(prev_lev);
					blocknum.push_back(b[k]);
					rpi.push_back(r[k]);
					hash.push_back(h[k]);
				}	
				r.clear();
				b.clear();
				h.clear();
				prev_lev = cur_lev;
			}
			cout<<v.first;
			for(auto iter = v.second.begin(); iter != v.second.end(); iter++)
			{
				cout <<" : "<< iter->first << "," << iter->second.data() << std::endl;
				if(v.first.find("blocknum") != string::npos) {
					b.push_back(stoi(iter->second.data()));
				} else if(v.first.find("rpi") != string::npos) {
					r.push_back(convertToDecimal(iter->second.data(), this->order));
				} else if (v.first.find("hash") != string::npos) {
					h.push_back(iter->second.data());	
				}
			}		
		}
	}
	for(int k = 0; k < r.size(); k++) {
		level.push_back(prev_lev);
		blocknum.push_back(b[k]);
		rpi.push_back(r[k]);
		hash.push_back(h[k]);
	}

	sortByLevel();
	/*	
		declare @rpid as int;
		select @rpid = RPI from Employee where ID = 40;
		select RPI, hash from Employee where RPI >= (@rpid/3)*3 and RPI < (@rpid/3)*3+3;
		select RPI, hash from Emp_1 where RPI >= (@rpid/(3*3))*3 and RPI < (@rpid/(3*3))*3+3;
		select RPI, hash from Emp_2 where RPI >= (@rpid/(3*3*3))*3 and RPI < (@rpid/(3*3*3))*3+3;	
		return n;
	*/
}

void RpiAuth::sortByLevel()
{
	int min_idx = 0; 
	for(int i = 0; i < level.size(); i++) {
		min_idx = i;
		for(int j = i+1; j < level.size(); j++) {
			if (level[j] < level[min_idx]) 
				min_idx = j;	
		}
		if(min_idx != i) {
			iter_swap(level.begin() + min_idx, level.begin() + i);
			iter_swap(blocknum.begin() + min_idx, blocknum.begin() + i);
			iter_swap(rpi.begin() + min_idx, rpi.begin() + i);
			iter_swap(hash.begin() + min_idx, hash.begin() + i);
		}
	}
}

bool RpiAuth::hasSiblings(int cur_rpi, int test_rpi, int lev, vector<int>& rpi_list)
{
	bool ret = false;
	int cur_parent_idx = cur_rpi/this->order;
	int test_parent_idx = test_rpi/this->order;
	int low_bound = (cur_rpi / pow(this->order, lev))*(this->order);
	int up_bound = low_bound + (this->order);
	if(test_parent_idx >= low_bound && test_parent_idx <= up_bound) {
		int start_idx = find(level.begin(), level.end(), (lev - 1)) - level.begin();
		int end_idx = find(level.rbegin(), level.rend(), (lev - 1)) - level.rend();
		end_idx = -end_idx;
		for(int i = low_bound; i <= up_bound; i++) {
			auto found_idx = find((rpi.begin() + start_idx), (rpi.begin() + end_idx), i);
			if(found_idx != (rpi.begin() + end_idx)) { // found
				int s_idx = find(level.begin(), level.end(), lev) - level.begin();
				int e_idx = find(level.rbegin(), level.rend(), lev) - level.rend();
				e_idx = -e_idx;
				auto find_child = find((rpi.begin() + s_idx), (rpi.begin() + e_idx), i*this->order);
				if(find_child != rpi.begin() + e_idx) {
					rpi_list.push_back(i*this->order);
				}
			}
		} 
	}
	if(rpi_list.size() > 0) {
		return true;
	}	
	return false;
}

bool RpiAuth::validate(vector<int> b, vector<string> h, string& topHash)
{
	if(b.size() == 0 || level.size() == 0) {
		cout<<"Validation data not enough\n";
		return false;
	}	

	vector<int> b_rpi(b.size(), -1);
	vector<int> b_idx(b.size(), -1);
	int start_idx = find(level.begin(), level.end(), (this->levels - 1)) - level.begin();
	int end_idx = find(level.rbegin(), level.rend(), (this->levels - 1)) - level.rend();
	end_idx = -end_idx;
	int isValid = true;
	for(int i = 0; i < b.size(); i++) {
		for (int j = start_idx; j < end_idx; j++) {
			if(blocknum[j] == b[i]) {
				b_rpi[i] = rpi[j];		
				b_idx[i] = j;
				if(hash[j] != h[i]) {
					cout<<"Invalid hash!!\n";
				 	return false;
				}
				break;
			}
		}	
	}
	
	if(b_rpi.size() == 0) {
		cout<<"Invalid blocknum\n";
		return false;
	}

	int cur_rpi = 0;
	unsigned char r_hash[SHA256_DIGEST_LENGTH] = {0};
	for(int i = 0; i < b.size(); i++) {
		cur_rpi = b_rpi[i];
		unsigned char c_hash[SHA256_DIGEST_LENGTH] = {0};
		str_to_uchar(h[i], c_hash);	
		//memcpy(c_hash, h[i], SHA256_DIGEST_LENGTH);
		unsigned char prev_hash[SHA256_DIGEST_LENGTH] = {0};
		unsigned char level_hash[SHA256_DIGEST_LENGTH] = {0};
		int low_bound = 0, up_bound = 0;
		for(int j = this->levels - 1, k = 1; j >= 0 && k <= this->levels - 1; j--, k++) {
			int start_idx = find(level.begin(), level.end(), j)  - level.begin();
			int end_idx = find(level.rbegin(), level.rend(), j) - level.rend();
			end_idx = -end_idx;
			bool first = true;
			low_bound = (cur_rpi / pow(this->order, k))*(this->order);
			up_bound = low_bound + this->order;
			//int start_idx = 0, end_idx = 0;
			for(int l = start_idx ; l < end_idx; l++) {
				if(level[l] == this->levels - 1) { // Is leaf
					vector<int> rpi_list;
					bool isSibling = hasSiblings(rpi[l], cur_rpi, level[l], rpi_list);
					if(isSibling && find(rpi_list.begin(), rpi_list.end(), rpi[l]) != rpi_list.end()) {
						if(first) {
							if(rpi[l] == cur_rpi) {
								str_to_uchar(h[i], r_hash);
							} else {
								str_to_uchar(hash[l], r_hash);
							}
							first = false;
						} else {
							if(rpi[l] == cur_rpi) {
								str_to_uchar(h[i], c_hash);
							} else {
								str_to_uchar(h[l], c_hash);
							}
							hash_add(r_hash, c_hash, r_hash); 
						}	
					}
				} else {
					unsigned char temp_hash[SHA256_DIGEST_LENGTH] = {0};
					str_to_uchar(hash[l], temp_hash);
					if(rpi[l] >= low_bound && rpi[l] <= up_bound) { 
						if(first) {
							if(rpi[l] == cur_rpi) { 
								memcpy(prev_hash, level_hash, SHA256_DIGEST_LENGTH);
								memcpy(r_hash, level_hash, SHA256_DIGEST_LENGTH);
							} else {
								memcpy(r_hash, temp_hash, SHA256_DIGEST_LENGTH);
								memcpy(prev_hash, temp_hash, SHA256_DIGEST_LENGTH);
							}
							first = false;
						} else {
							if(rpi[l] == cur_rpi) { 
								if(memcmp(prev_hash, level_hash, SHA256_DIGEST_LENGTH) == 0) {
									continue;	
								} else {
									hash_add(r_hash, level_hash, r_hash);
									memcpy(prev_hash, r_hash, SHA256_DIGEST_LENGTH);
								}
							} else {
								if(memcmp(prev_hash, temp_hash, SHA256_DIGEST_LENGTH) == 0) {
									continue;
								} else {
									hash_add(r_hash, temp_hash, r_hash);
									memcpy(prev_hash, temp_hash, SHA256_DIGEST_LENGTH);
								}
							}
						}
					} else {
						if(first) {
							memcpy(prev_hash, temp_hash, SHA256_DIGEST_LENGTH);
							memcpy(r_hash, temp_hash, SHA256_DIGEST_LENGTH);
							first = false;
						} else if (memcmp(prev_hash, temp_hash, SHA256_DIGEST_LENGTH) == 0) {
							continue;
						} else {
							hash_add(r_hash, temp_hash, r_hash);
							memcpy(prev_hash, temp_hash, SHA256_DIGEST_LENGTH);
						}
					}
				}
			}
			cur_rpi = cur_rpi/this->order;
			memcpy(level_hash, r_hash, SHA256_DIGEST_LENGTH);
		}
		if(i == 0) {
			topHash = uchar_to_str(r_hash);
		} else {
			string temp = uchar_to_str(r_hash);
			if(temp != topHash) {
				return false;
			}
		}
	}
		
	return true;
}
