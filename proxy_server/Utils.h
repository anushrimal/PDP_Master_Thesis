#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <string.h>
#include <openssl/sha.h>

#define DEFAULT_BLOCKSIZE 65536
#define MIN_BLOCK_COUNT 8
#define MAX_BLOCK_COUNT 24

using namespace std;
// TYPES.
typedef struct record {
	int blocknum;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	int rpi;
	int level;
	int index;
} record;

typedef struct node {
	void ** pointers;
	int * keys;
	struct node * parent;
	bool is_leaf;
	int num_keys;
	struct node * next; // Used for queue.
        int level;
	int * rpi;
	int * blocknum;
	int * nodeCnt;
	unsigned char hash[SHA256_DIGEST_LENGTH];
} node;

typedef struct auth_table {
	int level;
	int blocknum;
	int rpi;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	
	auth_table(int l, int b, int r, unsigned char h[SHA256_DIGEST_LENGTH]):level(l), blocknum(b), rpi(r)
	{
		memcpy(hash, h, SHA256_DIGEST_LENGTH);
	}
} auth_table;

// Util functions

string uchar_to_str(unsigned char hash[SHA256_DIGEST_LENGTH]);

void str_to_uchar(string str, unsigned char (&hash)[SHA256_DIGEST_LENGTH]);

int basal_sum(int base, int a, int b);

int basal_product(int base, int a, int b);

long convertToDecimal(string &input, int base);

void sha256(char *memblock, int sz, unsigned char (&hash)[SHA256_DIGEST_LENGTH]);

void hash_add(unsigned char rec1[SHA256_DIGEST_LENGTH], unsigned char rec2[SHA256_DIGEST_LENGTH], unsigned char (&hash_sum)[SHA256_DIGEST_LENGTH]);

int getBlockSize(int fileLenth);

#endif
