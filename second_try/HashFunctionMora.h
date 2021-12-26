#include <iostream>
#include <cstring>

static const uint8_t PI[16] = { 15, 9, 1, 7, 13, 12, 2, 8, 6, 5, 14, 3, 0, 11, 4, 10 };
static const uint8_t TAU[16] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
static const uint16_t A[16] = { 0x3a22, 0x8511, 0x4b99, 0x2cdd,
						0x483b, 0x248c, 0x1246, 0x9123,
						0x59e5, 0xbd7b, 0xcfac, 0x6e56,
						0xac52, 0x56b1, 0xb3c9, 0xc86d };

static const uint8_t C[8][8] = {
	{ 0xc0, 0x16, 0x46, 0x33, 0x57, 0x5a, 0x96, 0x99 },
	{ 0x92, 0x5b, 0x4e, 0xf4, 0x9a, 0x5e, 0x71, 0x74 },
	{ 0x86, 0xa8, 0x9c, 0xdc, 0xf6, 0x73, 0xbe, 0x26 },
	{ 0x18, 0x85, 0x55, 0x8f, 0x0e, 0xac, 0xa3, 0xf1 },
	{ 0xdc, 0xfc, 0x5b, 0x89, 0xe3, 0x5e, 0x84, 0x39 },
	{ 0x54, 0xb9, 0xed, 0xc7, 0x89, 0x46, 0x4d, 0x23 },
	{ 0xf8, 0x0d, 0x49, 0xaf, 0xde, 0x04, 0x4b, 0xf9 },
	{ 0x8c, 0xbb, 0xdf, 0x71, 0xcc, 0xaa, 0x43, 0xf1 }
};

class HashFunctionMora
{
public:
	HashFunctionMora(int message_length);
	HashFunctionMora(int message_length, const int HASH_LEN);
	~HashFunctionMora();
	void calculate_hash(uint8_t* data, uint8_t* res);
	void gN(uint8_t* h, const uint8_t* m, const uint8_t* N);
	void print_hash(const uint8_t* result);
	int get_HASH_LEN();

private:
	uint8_t* N;
	uint8_t* sigma;
	uint8_t* v_0;
	uint8_t* v_64;
	int message_len;
	int HASH_LEN;

	void X(const uint8_t* a, const uint8_t* b, uint8_t* res);
	void add64(uint8_t* a, uint8_t* b, uint8_t* res);
	void S(uint8_t* block);
	void P(uint8_t* block);
	void L(uint8_t* block);
	void get_key(uint8_t* K, int i);
	void E(uint8_t* K, const uint8_t* m, uint8_t* res);
	void hash(uint8_t* h, uint8_t* m);
	void padding(uint8_t* m);
	void int_to_arr(int index, uint8_t* message_int);
	void hash_appendix(uint8_t* h, uint8_t* m);
	void padd_to_8(uint8_t* h, uint8_t* res);
};
