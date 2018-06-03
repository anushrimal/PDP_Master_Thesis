#ifndef _REST_SERVER_H_
#define _REST_SERVER_H_

#include<string>
#include<iostream>
#include<vector>

#include "restbed"
#include "DBHandler.h"
#include "SendFile.h"

using namespace std;
using namespace restbed;

class RestServer 
{
public:
	enum SaveOption
	{
		ON_DB,
		ON_CLUSTER
	};

	RestServer(int portNum, SaveOption opt);

	static bool setDBPath(string dbPath);

	static bool setClusterDetails(multimap<string, unsigned int> cns);

	static void start();

	static void postMethodHandler( const shared_ptr< Session > session );

	static void getMethodHandler( const shared_ptr< Session > session );

	static void readChunkSize( const shared_ptr< Session > session, const Bytes& data );

	static void readChunk( const shared_ptr< Session > session, const Bytes& data );
protected:
	static SaveOption mSaveOpt;
	static int mPortNum;
	static DBHandler *mDB;
	static vector<SendFile *> mSendFiles;
	static string mBoundary;

};	

#endif
