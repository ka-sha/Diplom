#include <iostream>

class HashFunctionMora
{
public:
	HashFunctionMora(int message_length);
	uint8_t* calculate_hash(uint8_t* data);
	uint8_t* gN(uint8_t* h, uint8_t* m, uint8_t* N);
	void print_array(const uint8_t* result);

private:
	uint8_t* N;
	uint8_t* sigma;
	uint8_t* v_0;
	uint8_t* v_64;
	int message_len;

	uint8_t* X(const uint8_t* a, const uint8_t* b);
	uint8_t* add64(uint8_t* a, uint8_t* b);
	uint8_t* S(uint8_t* block);
	uint8_t* P(uint8_t* block);
	uint8_t* L(uint8_t* block);
	uint8_t* get_key(uint8_t* K, int i);
	uint8_t* E(uint8_t* K, uint8_t* m);
	uint8_t* hash(uint8_t* h, uint8_t* m);
	uint8_t* padding(uint8_t* m);
	uint8_t* int_to_arr(int index);
	uint8_t* hash_appendix(uint8_t* h, uint8_t* m);
};