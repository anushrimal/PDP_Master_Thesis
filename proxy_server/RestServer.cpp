#include <memory>
#include <cstring>
#include <cstdlib>
#include <ciso646>
#include <iostream>
#include <fstream>  
#include <errno.h>

#include <boost/algorithm/string.hpp>
#include "restbed"
#include "RestServer.h"

RestServer::SaveOption RestServer::mSaveOpt;
int RestServer::mPortNum;
DBHandler* RestServer::mDB;
SendFile*  RestServer::mSendFile;
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
	auto settings = make_shared< Settings >( );
	settings->set_port(mPortNum );
	settings->set_default_header( "Connection", "close" );

	cout<<"Starting server on port number "<<mPortNum<<endl;
	Service service;
	service.publish( one );
	service.publish( two );
	service.start( settings );
}
	
bool RestServer::setDBPath(string dbPath) {
	if(mSaveOpt != ON_DB) 
		return false;

	mDB = new DBHandler();
	bool stat = mDB->init(dbPath);
	if(stat == false)
		return false;
	
	return true;
}

bool RestServer::setClusterDetails(char *ip, int port)
{
	if(mSaveOpt != ON_CLUSTER)
		return false;
	mSendFile = new SendFile(ip, port);
	return true;
}

void RestServer::getMethodHandler( const shared_ptr< Session > session )
{
	const auto& request = session->get_request( );
	const string body = request->get_path_parameter( "file" );
	string fileDat;
	cout<<"Get Request :"<<body<<endl;
	if(mSaveOpt == ON_CLUSTER) {
		if(mSendFile) {
			fileDat = mSendFile->getFile(body.c_str());
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
			const auto request = session->get_request( );
			const auto body = request->get_body( );

			fprintf( stdout, "Complete body content: %.*s\n", static_cast< int >( body.size( ) ), body.data( ) );
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
	session->close( OK );
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
	cout<<"Content length :"<<content.length()<<endl;
	//fprintf( stdout, "Complete body content: %.*s\n", static_cast< int >( body.size( ) ), body.data( ) );
	size_t pos = 0;
	string token;
	string fileName;
	string fileData;
	int contentLen = 0;
	bool fileNameFound = false;
	bool fileDataFound = false;
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
		if(!fileDataFound) {
			int contentStart = 0, contentEnd = 0;
			if((index = token.find("name=\"FileData\"")) != std::string::npos) {
				token = (token.substr(index + 15, token.length()));
				//if(mSaveOpt != ON_DB) {
					int contentLenStart = token.find("\r\n\r\n\r\n") + 6;
					int contentLenEnd = token.find("\r\n", contentLenStart);
					string contentLenStr = token.substr(contentLenStart, contentLenEnd - contentLenStart);
                               		contentLen = strtoul(contentLenStr.c_str(), NULL, 16);
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
	cout<<"Content length after erasing :"<<fileData.length()<<endl;
	if(mSaveOpt == ON_CLUSTER) {
		if(mSendFile) {
			myfile.open (fileName);
			myfile << fileData;
			myfile.close();
			mSendFile->sendFile(fileName.c_str(), "example.txt");
		}
	} else if(mSaveOpt == ON_DB) {
		mDB->writeFile(fileName.c_str(),  fileData.c_str());
		//cout<<"File saved on DB as:\n\t"<<mDB->readFile(fileName.c_str())<<endl;
	}
}

void RestServer::readChunk( const shared_ptr< Session > session, const Bytes& data )
{
	static int chunkNo = 1;
	cout << "Partial body chunk: " << data.size( ) << " bytes" << endl;
	session->fetch( "\r\n", RestServer::readChunkSize );
}

