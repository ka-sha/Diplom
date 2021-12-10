#include <iostream>

class HashFunctionMora
{
public:
	HashFunctionMora(int);
	uint8_t* calculate_hash(uint8_t*);
	uint8_t* gN(uint8_t*, uint8_t*, uint8_t*);
	void print_array(uint8_t*);

private:
	uint8_t* N;
	uint8_t* sigma;
	uint8_t* v_0;
	uint8_t* v_64;
	int message_len;

	uint8_t* X(const uint8_t*, const uint8_t*);
	uint8_t* add64(uint8_t*, uint8_t*);
	uint8_t* S(uint8_t*);
	uint8_t* P(uint8_t*);
	uint8_t* L(uint8_t*);
	uint8_t* get_key(uint8_t*, int i);
	uint8_t* E(uint8_t*, uint8_t*);
	uint8_t* hash(uint8_t*, uint8_t*);
	uint8_t* padding(uint8_t*);
	uint8_t* int_to_arr(int);
	uint8_t* hash_appendix(uint8_t*, uint8_t*);
};