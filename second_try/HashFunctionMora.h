#include <iostream>

class HashFunctionMora
{
public:
	HashFunctionMora(int message_length);
	~HashFunctionMora();
	void calculate_hash(uint8_t* data, uint8_t* res);
	void gN(uint8_t* h, uint8_t* m, uint8_t* N);
	void print_array(const uint8_t* result);

private:
	uint8_t* N;
	uint8_t* sigma;
	uint8_t* v_0;
	uint8_t* v_64;
	int message_len;

	void X(const uint8_t* a, const uint8_t* b, uint8_t* res);
	void add64(uint8_t* a, uint8_t* b, uint8_t* res);
	void S(uint8_t* block);
	void P(uint8_t* block);
	void L(uint8_t* block);
	void get_key(uint8_t* K, int i);
	void E(uint8_t* K, uint8_t* m, uint8_t* res);
	void hash(uint8_t* h, uint8_t* m);
	void padding(uint8_t* m);
	void int_to_arr(int index, uint8_t* message_int);
	void hash_appendix(uint8_t* h, uint8_t* m);
};