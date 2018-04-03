
#include <string>
#include <iostream>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "DBHandler.h"

DBHandler::~DBHandler()
{
	delete mDb; 
}

bool DBHandler::init(string dbPath) {
	mDbPath = dbPath;
	Options options;
	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();
	options.create_if_missing = true;
	Status s = DB::Open(options, mDbPath, &(this->mDb));
	return s.ok();
}

bool DBHandler::writeFile(const char *fileName, const char *fileData)
{
	Status s;
	s = this->mDb->Put(WriteOptions(), fileName, fileData);
	return s.ok();
}
	
string DBHandler::readFile(const char *fileName)
{
	string retVal;
	Status s = this->mDb->Get(ReadOptions(), fileName, &retVal);
	if(s.ok()) 
		cout<<"Read Successful"<<endl;
		
	return retVal;
} 
