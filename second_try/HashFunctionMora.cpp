#include <iostream>
#include "HashFunctionMora.h"

HashFunctionMora::HashFunctionMora(int message_length)
{
	N = new uint8_t[8];
	sigma = new uint8_t[8];
	v_0 = new uint8_t[8];
	v_64 = new uint8_t[8];
	std::fill_n(N, 8, 0);
	std::fill_n(sigma, 8, 0);
	std::fill_n(v_0, 8, 0);
	std::fill_n(v_64, 8, 0);
	v_64[7] = 0x40;
	message_len = message_length;
}

HashFunctionMora::~HashFunctionMora()
{
	delete[] N;
	delete[] sigma;
	delete[] v_0;
	delete[] v_64;
}

void HashFunctionMora::calculate_hash(uint8_t* data, uint8_t* res)
{
	uint8_t* tmp_res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	while (message_len >= 8)
	{
		hash(tmp_res, data);
		data += 8;
		message_len -= 8;
	}
	hash_appendix(tmp_res, data);
	
	memcpy(res, tmp_res, 8);
	delete[] tmp_res;
}

void HashFunctionMora::hash(uint8_t* h, uint8_t* m)
{
	gN(h, m, N);
	add64(N, v_64, N);
	add64(sigma, m, sigma);
}

void HashFunctionMora::gN(uint8_t* h, uint8_t* m, uint8_t* N)
{
	uint8_t* K = new uint8_t[8];
	uint8_t* tmp_res = new uint8_t[8];

	X(h, N, K);
	S(K);
	P(K);
	L(K);

	E(K, m, tmp_res);

	X(tmp_res, h, tmp_res);
	X(tmp_res, m, h);

	delete[] K;
	delete[] tmp_res;
}

void HashFunctionMora::X(const uint8_t* a, const uint8_t* b, uint8_t* res)
{
	for (int i = 0; i < 8; i++)
	{
		res[i] = a[i] ^ b[i];
	}
}

void HashFunctionMora::S(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];
	uint8_t left_state, right_state;

	for (int i = 0; i < 8; i++)
	{
		left_state = block[i] >> 4;
		right_state = block[i] & 0xf;
		res[i] = (PI[left_state] << 4) ^ PI[right_state];
	}
	memcpy(block, res, 8);
	delete[] res;
}

void HashFunctionMora::P(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);
	uint8_t index, block_index, block_value;

	for (int i = 0; i < 16; i++) {
		index = TAU[i];
		block_index = index / 2;
		block_value = (index % 2 == 0) ? block[block_index] >> 4 : block[block_index] & 0xf;
		res[i / 2] ^= (i % 2 == 0) ? (block_value << 4) : block_value;
	}
	memcpy(block, res, 8);
	delete[] res;
}

void HashFunctionMora::L(uint8_t* block)
{
	uint16_t* tmp_res = new uint16_t[4];
	std::fill_n(tmp_res, 4, 0);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			if (j < 8)
			{
				if (block[2 * i] & (1 << j))
				{
					tmp_res[i] ^= A[7 - j];
				}
			}
			else
			{
				if (block[2 * i + 1] & (1 << (j - 8)))
				{
					tmp_res[i] ^= A[15 - (j - 8)];
				}
			}
		}
	}

	uint8_t* res = new uint8_t[8];
	for (int i = 0; i < 8; i++)
	{
		if (i % 2)
		{
			res[i] = tmp_res[i / 2] & 0xff;
		}
		else
		{
			res[i] = tmp_res[i / 2] >> 8;
		}
	}

	memcpy(block, res, 8);
	delete[] tmp_res;
	delete[] res;
}

void HashFunctionMora::E(uint8_t* K, uint8_t* m, uint8_t* res)
{
	X(K, m, res);

	for (int i = 0; i < 8; i++)
	{
		S(res);
		P(res);
		L(res);
		get_key(K, i);
		X(K, res, res);
	}
}

void HashFunctionMora::get_key(uint8_t* K, int i)
{
	X(K, C[i], K);
	S(K);
	P(K);
	L(K);
}

void HashFunctionMora::add64(uint8_t* a, uint8_t* b, uint8_t* res)
{
	int internal = 0;
	for (int i = 7; i >= 0; i--)
	{
		internal = a[i] + b[i] + (internal >> 8);
		res[i] = internal & 0xff;
	}
}

void HashFunctionMora::hash_appendix(uint8_t* h, uint8_t* m)
{
	uint8_t* message_int = new uint8_t[8];

	padding(m);
	gN(h, m, N);
	int_to_arr(message_len, message_int);
	add64(N, message_int, N);
	add64(sigma, m, sigma);
	gN(h, N, v_0);
	gN(h, sigma, v_0);

	delete[] message_int;
}

void HashFunctionMora::padding(uint8_t* m)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);
	//message_len from 0 to 7
	int i = 7 - message_len;
	
	res[i++] = 0x01;
	for (int j = i; j < 8; j++)
	{
		res[j] = m[j];
	}
	memcpy(m, res, 8);
	delete[] res;
}

void HashFunctionMora::int_to_arr(int index, uint8_t* res)
{
	res[7] = (uint8_t) (2 ^ index);
}

void HashFunctionMora::print_array(const uint8_t* result)
{
	for (int i = 0; i < 8; i++)
	{
		printf("%02x", result[i]);
	}
	std::cout << std::endl;
}