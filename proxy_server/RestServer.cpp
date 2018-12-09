#include <memory>
#include <cstring>
#include <cstdlib>
#include <ciso646>
#include <iostream>
#include <fstream>  
#include <errno.h>
#include <thread>

#include <boost/algorithm/string.hpp>
#include "restbed"
#include "RestServer.h"
#include "BPlusTree.h"
#include "RpiAuth.h"

#define DEFAULT_ORDER 3

RestServer::SaveOption RestServer::mSaveOpt;
int RestServer::mPortNum;
DBHandler* RestServer::mDB;
vector<SendFile*>  RestServer::mSendFiles;
string RestServer::mBoundary;

RestServer::RestServer(int portNum, SaveOption opt)
{
	mPortNum = portNum;
	mSaveOpt = opt;
}
void RestServer::start()
{
	auto one = make_shared< Resource >( );
	one->set_path( "/resources" );
	one->set_method_handler( "POST", postMethodHandler );

	auto two = make_shared< Resource >( );
    	two->set_path( "/resources/{file: .*}" );
    	two->set_method_handler( "GET", getMethodHandler);

	auto three =  make_shared< Resource >( );
	three->set_path( "/challenge/{file: .*}" );
	three->set_method_handler( "CHALLENGE", challengeMethodHandler);

	auto settings = make_shared< Settings >( );
	settings->set_port(mPortNum );
	settings->set_worker_limit( std::thread::hardware_concurrency( ) );
	settings->set_default_header( "Connection", "close" );

	cout<<"Starting server on port number "<<mPortNum<<endl;
	Service service;
	service.publish( one );
	service.publish( two );
	service.publish( three );
	service.start( settings );
}
	
bool RestServer::setDBPath(string dbPath) {
	mDB = new DBHandler();
	bool stat = mDB->init(dbPath);
	if(stat == false)
		return false;
	
	return true;
}


bool RestServer::setClusterDetails(multimap<string, unsigned int> cns)
{
	if(mSaveOpt != ON_CLUSTER)
		return false;
	for(auto it = cns.begin(); it !=  cns.end(); ++it) {
		mSendFiles.push_back(new SendFile(it->first, it->second));
	}
	//mSendFile = new SendFile(ip, port);
	return true;
}

void RestServer::challengeMethodHandler( const shared_ptr< Session > session )
{
	const auto& request = session->get_request( );
	const string body = request->get_path_parameter( "file" );
	string fileDat;
	if(mSaveOpt == ON_CLUSTER) {
		RpiAuth *rpiA = new RpiAuth();
		if(rpiA->parse(mDB, body.c_str())) {
			set<int> bnums;
			int blockcount = rpiA->blockcount;
			int blocksize = rpiA->blocksize;
			//Select 20% of blocks to verify
			double blockperc = 0;
			vector<int> blocknums;
			while(blockperc < 20) {
				auto ret = bnums.insert(rand()%	blockcount);
				if(ret.second) { // inserted
					if(bnums.find(0) != bnums.end()) {
						bnums.erase(0);
						continue;
					}
					blockperc += 1/(double)blockcount * 100;
				}
			}
			if(!mSendFiles.empty()) {
				int cn_id = rand()%mSendFiles.size();
				stringstream blocks; 
				//Add blocks in format "start_idx1:end_idx1,start_idx2:end_idx2 ...."
				for(auto block : bnums) {
					int start_idx = (block == 1)?0:(block - 1) *blocksize;
					int end_idx = start_idx + blocksize - 1;
					blocks<<start_idx;
					blocks<<":";
					blocks<<end_idx; 	
					blocks<<",";
				}
				string params;
				blocks>>params;
				string response = mSendFiles[cn_id]->challenge(body.c_str(), params);
				vector<string> hashes;
				boost::algorithm::split(hashes, response, boost::algorithm::is_any_of(","));
				string chalRes;
				copy(bnums.begin(), bnums.end(), std::back_inserter(blocknums));
				if(rpiA->validate(blocknums, hashes, chalRes)) {
					session->close( OK, chalRes, { { "Content-Length", ::to_string( chalRes.size( ) ) } } );	
				} else {
					session->close( OK, chalRes);
				}
			}
		}
		
		} else {
			string idxs = request->get_query_parameter( "idx" );

			vector<string> blocks;
			fileDat = mDB->readFile(body.c_str());
			boost::algorithm::split(blocks, idxs, boost::algorithm::is_any_of(","));
			stringstream ss;
			for(int i = 0; i < blocks.size(); i++) {
				unsigned char c_hash[SHA256_DIGEST_LENGTH] = {0};
				int idx = blocks[i].find(":");	
				if(idx == -1) 
					continue;
				int start_idx = atoi(blocks[i].substr(0, idx).c_str());
				int end_idx = atoi(blocks[i].substr(idx+1, blocks[i].size()).c_str());
				if(end_idx > fileDat.size()) 
					end_idx = fileDat.size();
				sha256(fileDat.substr(start_idx, end_idx).c_str(), end_idx - start_idx + 1, c_hash);
				ss<<uchar_to_str(c_hash)<<',';
			}	
			string chalRes;
			ss>>chalRes;
			session->close( OK, chalRes, { { "Content-Length", ::to_string( chalRes.size( ) ) } } );
		
	}
}

void RestServer::getMethodHandler( const shared_ptr< Session > session )
{
	const auto& request = session->get_request( );
	const string body = request->get_path_parameter( "file" );
	string fileDat;
	cout<<"Get Request :"<<body<<endl;
	if(mSaveOpt == ON_CLUSTER) {
		if(!mSendFiles.empty()) {
			int cn_id = rand()%mSendFiles.size();
			fileDat += mSendFiles[cn_id]->getFile(body.c_str());
		}
	} else {
		fileDat = mDB->readFile(body.c_str());	
	}
	session->close( OK, fileDat, { { "Content-Length", ::to_string( fileDat.size( ) ) } } );
}


void RestServer::postMethodHandler( const shared_ptr< Session > session )
{
	const auto request = session->get_request( );
	
	string contentType = request->get_header( "Content-Type");
	mBoundary = contentType.substr( (contentType.find("boundary=") + 9), contentType.length());
	cout<<"Boundary :"<<mBoundary<<endl;
	if ( request->get_header( "Transfer-Encoding", String::lowercase ) == "chunked" )
	{
		cout<<"In post Method Handler. Fetching chunk size\n";
		session->fetch( "\r\n", RestServer::readChunkSize );
	}
	else if ( request->has_header( "Content-Length" ) )
	{
		cout<<"In post Method Handler. Fetching content length\n";
		int length = request->get_header( "Content-Length", 0 );
		session->fetch( length, [ ]( const shared_ptr< Session > session, const Bytes& )
		{
			//const auto request = session->get_request( );
			//const auto body = request->get_body( );
			const Bytes data;
			RestServer::readChunkSize(session, data);

			//fprintf( stdout, "Complete body content: %.*s\n", static_cast< int >( body.size( ) ), body.data( ) );
			session->close( OK );
		} );
	}
	else
	{
		cout<<"In post Method Handler. Bad request\n";
		session->close( BAD_REQUEST );
	}
}

void RestServer::readChunkSize( const shared_ptr< Session > session, const Bytes& data )
{
	if ( not data.empty( ) )
	{
		const string length( data.begin( ), data.end( ) );
		if ( length not_eq "0\r\n" )
		{
			const auto chunk_size = stoul( length, nullptr, 16 ) + strlen( "\r\n" );
			session->fetch( chunk_size, RestServer::readChunk );
			return;
		}
	}
	const auto request = session->get_request( );
	const auto body = request->get_body( );
	string content;// = (char*)(body.data());

	ofstream myfile;
	//myfile.open ("example.txt");
	for (const auto &e : body) { 
		//myfile << e;
		content += e;
	}
  	//myfile.close();
	//cout<<"Content length :"<<content.length()<<endl;
	//fprintf( stdout, "Complete body content: %.*s\n", static_cast< int >( body.size( ) ), body.data( ) );
	size_t pos = 0;
	string token;
	string fileName;
	string fileData;
	int contentLen = 0;
	bool fileNameFound = false;
	bool fileDataFound = false;
	bool secureTransfer = false;
	while ((pos = content.find(mBoundary)) != std::string::npos) {
		token = content.substr(0, pos);
		int index = 0;
		
		if(!fileNameFound) {
			if((index = token.find("name=\"FileName\"")) != std::string::npos) {
				fileName = token.substr(index + 15, token.length());
				int fileNameStart = fileName.find("\r\n\r\n") + 4;
				int fileNameEnd = fileName.find("\r\n", fileNameStart);
				fileName = fileName.substr( fileNameStart, fileNameEnd - fileNameStart );
				token = token.find(fileNameEnd + 2);
				fileNameFound = true;
				cout<<"Filename :"<<fileName<<endl;
			}
		}
		if(!secureTransfer) {
			if((index = token.find("name=\"Secured\"")) != std::string::npos) {
				secureTransfer = true;
			}
		}
		if(!fileDataFound) {
TryAgain:
			int contentStart = 0, contentEnd = 0;
			if((index = token.find("name=\"FileData\"")) != std::string::npos) {
				token = (token.substr(index + 15, token.length()));
				//if(mSaveOpt != ON_DB) {
					int contentLenStart = token.find("\r\n\r\n\r\n") + 6;
					int contentLenEnd = token.find("\r\n", contentLenStart);
					string contentLenStr = token.substr(contentLenStart, contentLenEnd - contentLenStart);
                               		contentLen = strtoul(contentLenStr.c_str(), NULL, 16);
					//int fileDataEnd = token.find("\r\n30\r\n\r\n--", contentLenEnd + 2);
					int fileDataEnd = token.find("\r\n", contentLenEnd + 2);
					fileData = token.substr(contentLenEnd + 2, fileDataEnd - (contentLenEnd + 2));
					token = token.substr(fileDataEnd + 2);
				/*} else {
					contentStart = token.find("\r\n\r\n\r\n")+5;
					contentEnd = token.find("\r\n", contentStart);
					fileData = token.substr(contentStart, contentEnd - contentStart);
					token = token.substr(contentEnd + 2);
				}*/
				int furtherConStart = 0, furtherConEnd = 0;
 
				while((furtherConStart = token.find("\r\n", contentEnd)) != -1) {
					furtherConEnd = token.find("\r\n", furtherConStart + 2);
					if(furtherConEnd == std::string::npos || furtherConEnd == furtherConStart + 2) 
						break; 
					//string contentLenStr = token.substr(furtherConStart, furtherConEnd - furtherConStart);
					//contentLen = strtoul(contentLenStr.c_str(), NULL, 16);
					//int fileDataEnd = fileData.find("\r\n", furtherConEnd + 2);
					fileData +=  token.substr(furtherConStart + 2, furtherConEnd - furtherConStart - 2);
					token = token.substr(furtherConEnd + 2);
				}
				fileDataFound = true;		
			}
		} else {
			int fileDataStart = token.find("\r\n\r\n") + 4;
			int fileDataEnd = token.find("\r\n", fileDataStart);
			fileData += token.substr(fileDataStart, fileDataEnd - fileDataStart);
		}
		content.erase(0, pos + mBoundary.length());
	}
	if(!fileDataFound) {
		token = content;
		goto TryAgain;
	}
	cout<<"Content length after erasing :"<<fileData.length()<<endl;
	if(mSaveOpt == ON_CLUSTER) {
		if(!mSendFiles.empty()) {
			if(secureTransfer) {
				int blocksize = getBlockSize(fileData.length());
				// TODO : Calculate order
				int order = DEFAULT_ORDER;
				string rootHash = generateBTree(fileName, fileData, blocksize, order);
				
				session->close( OK, rootHash );
			}
			myfile.open ("transfer.tmp");
			myfile << fileData;
			myfile.close();
			vector<thread> workers;
    			int i = 0;
			for (auto it = mSendFiles.begin(); it != mSendFiles.end(); ++it) {
        			workers.push_back(std::thread([it, fileName]() 
        			{
					(*it)->sendFile(fileName.c_str(), "transfer.tmp");
        			}));
			}
    
			std::for_each(workers.begin(), workers.end(), [](thread &t) 
			{
				t.join();
			});
			//for(auto it = mSendFiles.begin(); it != mSendFiles.end(); ++it)
			//	(*it)->sendFile(fileName.c_str(), "transfer.tmp");
			remove("transfer.tmp");
			session->close( OK );
		}
	} else if(mSaveOpt == ON_DB) {
		mDB->writeFile(fileName.c_str(),  fileData.c_str());
		//cout<<"File saved on DB as:\n\t"<<mDB->readFile(fileName.c_str())<<endl;
		session->close( OK );
	}
}

void RestServer::readChunk( const shared_ptr< Session > session, const Bytes& data )
{
	static int chunkNo = 1;
	//cout << "Partial body chunk: " << data.size( ) << " bytes" << endl;
	session->fetch( "\r\n", RestServer::readChunkSize );
}

string RestServer::generateBTree(string filename, string fileData, int blocksize, int order)
{
	map<int,string> blockHashes;
	int filesz = fileData.length();
	BPlusTree *btree = new BPlusTree(order);
	node * root = NULL;
	unsigned char t_hash[SHA256_DIGEST_LENGTH] = {0};
	string ret;

	int remsz = 0, blocknum = 1, blockcount = 0;
	int readIdx = 0;
	remsz = filesz;

	if(filesz%blocksize== 0) {
		blockcount = filesz / blocksize;
	} else {
		blockcount = filesz / blocksize + 1;
	}

	while(remsz > 0) {
		unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
		if(remsz < blocksize) {
			sha256(fileData.substr(readIdx, remsz).c_str(), remsz, hash);
			remsz = 0;
			readIdx += remsz;
		} else {
			sha256(fileData.substr(readIdx, blocksize).c_str(), blocksize, hash);
			remsz -= blocksize;
			readIdx += blocksize;
		}
		if(blocknum == 1) {
			memcpy(t_hash, hash, SHA256_DIGEST_LENGTH);
		}
		record *rec = new record();
		rec->blocknum = blocknum;
		memcpy(rec->hash, hash, SHA256_DIGEST_LENGTH);
		
		root = btree->insert(root, blocknum, rec);
		blocknum++;
	}
	cout<<"\n\n========================================================================\n";
	cout<<"Evaluating merkel tree hashes\n========================================================================\n";
	ret = btree->evaluate(root);
	btree->generate_rpi(root);
	btree->dumpDataOnDB(mDB, filename.c_str(), root, blocksize, blocknum - 1); 
	btree->destroy_tree(root);
	delete btree;
	return ret;
}
