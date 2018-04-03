#ifndef _DB_HANDLER_H_
#define _DB_HANDLER_H_

#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"

using namespace rocksdb;
using namespace std;

class DBHandler
{
	string mDbPath;
	DB* mDb;

public:
	~DBHandler();

	bool init(string dbPath);

	bool writeFile(const char *fileName, const char *fileData);
		
	string readFile(const char *fileName);
};

#endif
