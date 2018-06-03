#include<iostream>
#include<stdio.h>
#include<stdlib.h>

#include "RestServer.h"
#include "ParseConfig.h"

using namespace std;

void print_usage(char *program)
{
	 printf("USAGE : \n\t%s <config_file_path>\n", program);
}

int main(int argc, char*argv[])
{
	bool ret = false;
	RestServer::SaveOption opt = RestServer::ON_DB;
	bool sendToServer = false;

	//Handle command line options
	if(argc != 2) {
		print_usage(argv[0]);
	}
	ParseConfig cfg;
	ret = cfg.parse(argv[1]);
	if(!ret) {
		return -1;
	}
	if(cfg.mMode == "ps") {
		opt = RestServer::ON_CLUSTER;
	} else {
		opt = RestServer::ON_DB;
	}
	
	RestServer *srv = new RestServer(cfg.mPortNum, opt);

	if(opt == RestServer::ON_CLUSTER) {
			srv->setClusterDetails(cfg.mCNs);
	} else if (opt == RestServer::ON_DB) {
			srv->setDBPath(cfg.mPath);
	} else {
		cout<<"Choose one of the two : Save on DB (-b) or Send to Server (-s)"<<endl;
		print_usage(argv[0]);
		return -1;
	}
	srv->start();
	//thread t1(&RestServer::init, &srv, port_num, db_path, opt);
	
	return 0;
}
