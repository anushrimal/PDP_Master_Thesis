#include<iostream>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include <bits/stdc++.h> 
#include <chrono>
#include<dirent.h>
#include<sstream>
#include<fstream>

#include "RestServer.h"
#include "Utils.h"

#define DEFAULT_BLOCKSIZE 16384
#define DEFAULT_ORDER 4

using namespace std;

void print_usage(char *program)
{
	 printf("USAGE : \n\t%s <dir_path>\n", program);
}
	
static RestServer *srv = new RestServer(7777, RestServer::ON_DB);

void orderTest(char* dir_str)
{
	DIR *dir;
	struct dirent *dirent;
	ostringstream fileBuffer;
	string fileData;
	int blocksize = DEFAULT_BLOCKSIZE;

	ofstream csvFile("order_test.csv", ofstream::out);
	
	// Write headers
	csvFile << "FileName,FileSize,BlockSize,Order,Time,NodeCount\n";
	
	
	for(int i = 3; i < 9; i++) {
		dir = opendir(dir_str);
		if (dir != NULL) {
			while (dirent = readdir(dir)) {
				if (strcmp(dirent->d_name, ".") == 0 ||
				    strcmp(dirent->d_name, "..") == 0) {
					continue;
				}

				string filePath(dir_str);
				filePath.append(dirent->d_name);
				ifstream fileStream(filePath, ifstream::in);
				fileBuffer.str("");
				fileBuffer.clear();
				fileBuffer << fileStream.rdbuf();
				fileData = fileBuffer.str();
				//blocksize = getBlockSize(fileData.size());
				//blocksize = 8192;
				cout<<"Evaluating file : "<< dirent->d_name << "order : "<<i<<endl;
				auto start = chrono::high_resolution_clock::now();
				int node_count = 0;
				srv->generateBTree(dirent->d_name, fileData, blocksize, i, node_count);
				auto end = chrono::high_resolution_clock::now();
				double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
				time_taken *= 1e-9; 
				cout<< endl;

				csvFile << dirent->d_name<< "," << fileData.size() << "," << blocksize << "," << i << "," ;
				csvFile << fixed << time_taken << setprecision(9);
				csvFile << "," << node_count << endl;
				fileData.clear();
				fileStream.close();
			}
		}	
		closedir(dir);
	}

	csvFile.close();
}

void blockSizeTest(char* dir_str)
{
	DIR *dir;
	struct dirent *dirent;
	ostringstream fileBuffer;
	string fileData;
	int blocksize;

	ofstream csvFile("blockSize_test.csv", ofstream::out);
	
	// Write headers
	csvFile << "FileName,FileSize,BlockSize,Order,Time,NodeCount\n";
	
	
	for(int i = 8; i <= 256; i = i*2) {
		dir = opendir(dir_str);
		if (dir != NULL) {
			while (dirent = readdir(dir)) {
				if (strcmp(dirent->d_name, ".") == 0 ||
				    strcmp(dirent->d_name, "..") == 0) {
					continue;
				}

				string filePath(dir_str);
				filePath.append(dirent->d_name);
				ifstream fileStream(filePath, ifstream::in);
				fileBuffer.str("");
				fileBuffer.clear();
				fileBuffer << fileStream.rdbuf();
				fileData = fileBuffer.str();
				int node_count = 0;

				auto start = chrono::high_resolution_clock::now();
				srv->generateBTree(dirent->d_name, fileData, i*1024, DEFAULT_ORDER, node_count);
				auto end = chrono::high_resolution_clock::now();
				double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
				time_taken *= 1e-9; 
				cout<< endl;

				csvFile << dirent->d_name<< "," << fileData.size() << "," << i*1024 << "," << DEFAULT_ORDER <<",";
				csvFile << fixed << time_taken << setprecision(9);
				csvFile << "," << node_count << endl;
				fileData.clear();
				fileStream.close();
			}
		}	
		closedir(dir);
	}

	csvFile.close();
}


int main(int argc, char*argv[])
{
	bool ret = false;
	bool sendToServer = false;

	//Handle command line options
	if(argc != 2) {
		print_usage(argv[0]);
	}
	
	srv->setDBPath("/home/anukriti/proxy_server/db_store/");
	orderTest(argv[1]);
	blockSizeTest(argv[1]);
	
	return 0;
}


