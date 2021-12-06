#include <iostream>
#include "HashFunctionMora.h"

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
	v_64[0] = 0x40;
	message_len = message_length;
}

uint8_t* HashFunctionMora::calculate_hash(uint8_t* data)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	while (message_len >= 8)
	{
		res = hash(res, data);
		data += 8;
		message_len -= 8;
	}
	res = hash_appendix(res, data);

	return res;
}

uint8_t* HashFunctionMora::hash(uint8_t* h, uint8_t* m)
{
	uint8_t* res = gN(h, m, N);
	N = add64(N, v_64);
	sigma = add64(sigma, m);

	return res;
}

uint8_t* HashFunctionMora::gN(uint8_t* h, uint8_t* m, uint8_t* N)
{
	return X(X(E(L(P(S(X(h, N)))), m), h), m);
}

uint8_t* HashFunctionMora::X(const uint8_t* a, const uint8_t* b)
{
	uint8_t* res = new uint8_t[8];

	for (int i = 0; i < 8; i++)
	{
		res[i] = a[i] ^ b[i];
	}

	return res;
}

uint8_t* HashFunctionMora::S(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];

	for (int i = 0; i < 8; i++)
	{
		const uint8_t left_state = block[i] >> 4;
		const uint8_t right_state = block[i] & 0xf;
		res[i] = (PI[left_state] << 4) ^ PI[right_state];
	}

	return res;
}

uint8_t* HashFunctionMora::P(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	for (int i = 0; i < 16; i++) {
		const uint8_t index = TAU[i];
		const uint8_t block_index = index / 2;
		const uint8_t block_value = (index % 2 == 0) ? block[block_index] >> 4 : block[block_index];
		res[i / 2] ^= (i % 2 == 0) ? (block_value << 4) : block_value;
	}

	return res;
}

uint8_t* HashFunctionMora::L(uint8_t* block)
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

	return res;
}

uint8_t* HashFunctionMora::E(uint8_t* K, uint8_t* m)
{
	uint8_t* res = X(K, m);
	uint8_t* Ki = new uint8_t[8];
	memcpy(Ki, K, 8);

	for (int i = 0; i < 8; i++)
	{
		res = L(P(S(res)));
		Ki = get_key(Ki, i);
		res = X(Ki, res);
	}

	return res;
}

uint8_t* HashFunctionMora::get_key(uint8_t* K, int i)
{
	return L(P(S(X(K, C[i]))));
}

uint8_t* HashFunctionMora::add64(uint8_t* a, uint8_t* b)
{
	uint8_t* res = new uint8_t[8];
	int internal = 0;

	for (int i = 0; i < 7; i++)
	{
		internal = a[i] + b[i] + (internal >> 8);
		res[i] = internal ^ 0xff;
	}

	return res;
}

uint8_t* HashFunctionMora::hash_appendix(uint8_t* h, uint8_t* m)
{
	uint8_t* tmp_m = padding(m);
	uint8_t* res = gN(h, tmp_m, N);
	N = add64(N, int_to_arr(message_len));
	sigma = add64(sigma, tmp_m);
	res = gN(res, N, v_0);
	res = gN(res, sigma, v_0);

	return res;
}

uint8_t* HashFunctionMora::padding(uint8_t* m)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	memcpy(res, m, message_len);
	res[message_len] = 1;

	return res;
}

uint8_t* HashFunctionMora::int_to_arr(int index)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	res[0] = (uint8_t) (2 ^ index);

	return res;
}

void HashFunctionMora::print_array(uint8_t* result)
{
	for (int i = 0; i < 8; i++)
	{
		printf("%02x", result[i]);
	}
	std::cout << std::endl;
}