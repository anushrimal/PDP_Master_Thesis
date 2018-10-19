#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include "Utils.h"

string uchar_to_str(unsigned char hash[SHA256_DIGEST_LENGTH])
{
        stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
                ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
}

void str_to_uchar(string str, unsigned char (&hash)[SHA256_DIGEST_LENGTH])
{
        unsigned char *ch = hash;
        stringstream ss;
        for (unsigned i = 0; i < str.length(); i += 2) {
                string s = str.substr(i, 2);
                unsigned long l = strtoul(s.c_str(), 0, 16);
                memcpy(ch++, &l, sizeof(unsigned char));
        }
}

int basal_sum(int base, int a, int b)     //a and b are numbers in given base
{
	int sum = 0, digit = 0, carry = 0, digit_rank = 1;

	// Calculate the sum
	while (a > 0 || b > 0 || carry)
	{
		// Calculate the digit
		digit = a % 10 + b % 10 + carry;

		// Determine if you should carry or not
		if (digit > (base - 1))
		{
		    carry = 1;
		    digit %= base;
		}
		else
		    carry = 0;

		// Add the digit at the beggining of the sum
		sum += digit * digit_rank;
		digit_rank *= 10;

		// Get rid of the digits of a and b we used
		a /= 10;
		b /= 10;
	}
	return sum;
}

int basal_product(int base, int a, int b)
{
	int product = 0, digit = 0, carry = 0, digit_rank = 1;
	int dig = 0;
	// Calculate the product
	while (a > 0 || carry)
	{
		// Calculate the digit
		digit = (a % 10 * b ) + carry;

		// Determine if you should carry or not
		if (digit > (base - 1))
		{
		    carry  = digit / base;
		    digit = digit % base;
		}
		else
		    carry = 0;

		// Add the digit at the beggining of the sum
		product += digit * digit_rank;
		digit_rank *= 10;

		// Get rid of the digits of a and b we used
		a /= 10;
	}
	return product;	
}

long convertToDecimal(string& input, int base)
{
    if(base < 2 || base > 10)
        return 0;
    
    bool isNegative = (input[0] == '-');    

    int startIndex = input.length()-1;
    int endIndex   = isNegative ? 1 : 0;
    
    long value = 0;
    int digitValue = 1;
    
    for(int i = startIndex; i >= endIndex; --i)
    {
        char c = input[i];
	c = c - '0';
        if(c >= base)
            return 0;
        
        // Get the base 10 value of this digit    
        value += c * digitValue;
        
        // Each digit has value base^digit position - NOTE: this avoids pow
        digitValue *= base;
    }
    
    return value;
}

void hash_add(unsigned char rec1[SHA256_DIGEST_LENGTH], unsigned char rec2[SHA256_DIGEST_LENGTH], unsigned char (&hash_sum)[SHA256_DIGEST_LENGTH])
{
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, rec1, SHA256_DIGEST_LENGTH);
	SHA256_Update(&ctx, rec2, SHA256_DIGEST_LENGTH);
	SHA256_Final(hash_sum, &ctx);
}

int getBlockSize(int fileLength)
{
	int blockSize = DEFAULT_BLOCKSIZE;
	int blockCount = fileLength/DEFAULT_BLOCKSIZE;
       	while(blockCount < MIN_BLOCK_COUNT || blockCount > MAX_BLOCK_COUNT) {
		if(blockCount < MIN_BLOCK_COUNT) {
			blockSize /= 2;
		} else {
			blockSize *= 2;
		}
		blockCount = fileLength/blockSize;
		if(blockSize == 0) {
			blockSize = 2;
			break;
		}
	}
	return blockSize;
}

void sha256(char *memblock, int sz, unsigned char (&hash)[SHA256_DIGEST_LENGTH])
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, memblock, sz);
    SHA256_Final(hash, &sha256);
}

