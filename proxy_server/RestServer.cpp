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
	auto resource = make_shared< Resource >( );
	resource->set_path( "/resources" );
	resource->set_method_handler( "POST", postMethodHandler );

	auto settings = make_shared< Settings >( );
	settings->set_port(mPortNum );
	settings->set_default_header( "Connection", "close" );

	cout<<"Starting server on port number "<<mPortNum<<endl;
	Service service;
	service.publish( resource );
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
	string content = (char*)(body.data());

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
				fileNameFound = true;
				cout<<"Filename :"<<fileName<<endl;
			}
		}
		if(!fileDataFound) {
			if((index = token.find("name=\"FileData\"")) != std::string::npos) {
				fileData = (token.substr(index + 15, token.length()));
				int contentLenStart = fileData.find("\r\n\r\n\r\n") + 6;
				int contentLenEnd = fileData.find("\r\n", contentLenStart);
				string contentLenStr = fileData.substr(contentLenStart, contentLenEnd - contentLenStart);
                               	contentLen = atoi(contentLenStr.c_str());
				int fileDataEnd = fileData.find("\r\n", contentLenEnd + 2);
				fileData = fileData.substr(contentLenEnd + 2, fileDataEnd - (contentLenEnd + 2));
				cout<<"FileData :"<<fileData<<endl;
				fileDataFound = true;		
			}
		} else {
			int fileDataStart = token.find("\r\n\r\n") + 4;
			int fileDataEnd = token.find("\r\n", fileDataStart);
			fileData += token.substr(fileDataStart, fileDataEnd - fileDataStart);
		}
		content.erase(0, pos + mBoundary.length());
	}
	if(mSaveOpt == ON_CLUSTER) {
		if(mSendFile) {
			mSendFile->sendFile(fileName.c_str(), fileData.c_str());
		}
	} else if(mSaveOpt == ON_DB) {
		mDB->writeFile(fileName.c_str(),  fileData.c_str());
		cout<<"File saved on DB as:\n\t"<<mDB->readFile(fileName.c_str())<<endl;
	}
}

void RestServer::readChunk( const shared_ptr< Session > session, const Bytes& data )
{
	cout << "Partial body chunk: " << data.size( ) << " bytes" << endl;

	session->fetch( "\r\n", RestServer::readChunkSize );
}

