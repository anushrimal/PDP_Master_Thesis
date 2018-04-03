#include<iostream>
#include<stdio.h>
#include<stdlib.h>

#include "RestServer.h"

using namespace std;

void print_usage(char *program)
{
	 printf("USAGE : \n\t%s <-d db-path> <-p port-number> <-b|-s> [-i send-to-ip-adress] [-r send-to-port-num]\n\t\t -d : Path where RocksDB must be initialized local disk\n\t\t-p : Port number where rest-server must listen\n\t\t-b : Store on DB\n\t\t-s : Send file to IP address\n", program);
}

int main(int argc, char*argv[])
{
	int optc = 0;
	int port_num = 0;
	int send_port_num = 0;
	char send_ip_addr[128] = {'\0'};
	char db_path[256] = {'\0'};
	bool ret = false;
	RestServer::SaveOption opt = RestServer::ON_DB;
	bool sendToServer = false;
	int argCount = 1;

	//Handle command line options
	if(argc < 3) {
		print_usage(argv[0]);
	}
	while (( optc = getopt(argc, argv,"d:p:i:r:bs")) != -1 ) {
		argCount +=  1;
		switch(  optc ) {
			case 'd': {
				strncpy(db_path, optarg, 256);
				argCount +=  1;
				break;
			}	
			case 'p': {
				if(!isdigit(*optarg)) {
					cout<<"Invalid arg"<<*optarg<<". Integer port number expected"<<endl;
					print_usage(argv[0]);
					return -1;
				}
				port_num = atoi(optarg);
				if(port_num < 0 || port_num > 65535) {
					cout<<"Invalid arg"<<*optarg<<". Port number should be between 0 and 65535"<<endl;
					return -1;
				}
				argCount +=  1;
				break;
			}
			case 'i': {
				strncpy(send_ip_addr, optarg, 128);
				argCount +=  1;
				break;
			}
			case 'r': {
				if(!isdigit(*optarg)) {
					cout<<"Invalid arg"<<*optarg<<". Integer port number expected"<<endl;
					print_usage(argv[0]);
					return -1;
				}
				send_port_num = atoi(optarg);
				if(send_port_num < 0 || send_port_num > 65535) {
					cout<<"Invalid arg"<<*optarg<<". Port number should be between 0 and 65535"<<endl;
					return -1;
				}
				argCount +=  1;
				break;
			}
			case 'b': {
				opt =  RestServer::ON_DB;
				break;
			}
			case 's': {
				opt =  RestServer::ON_CLUSTER;
				sendToServer = true;
				break;
			}
			default : {
				cout<<"Invalid arg"<<optc<<endl;
				print_usage(argv[0]);
				return -1;
			}
		}
	}
	if(port_num == 0) {
		cout<<"Invalid arg. Please provide a valid port-number"<<endl;
		print_usage(argv[0]);
		return -1;
	}
	RestServer *srv = new RestServer(port_num, opt);
	if(sendToServer) {
		if(send_port_num == 0 || send_ip_addr[0] == '\0') {
			cout<<"Invalid arg. Please provide valid IP address and port number"<<endl;
			print_usage(argv[0]);
			delete srv;
			return -1;
		} else {
			srv->setClusterDetails(send_ip_addr, send_port_num);
		}
	} else if (opt == RestServer::ON_DB) {
		if(db_path[0] == '\0') {
			cout<<"Invalid arg. Please provide a valid DB path"<<endl;
			print_usage(argv[0]);
			delete srv;
			return -1;
		}else {
			srv->setDBPath(db_path);
		}
	} else {
		cout<<"Choose one of the two : Save on DB (-b) or Send to Server (-s)"<<endl;
		print_usage(argv[0]);
		return -1;
	}
	srv->start();
	//thread t1(&RestServer::init, &srv, port_num, db_path, opt);
	
	return 0;
}
