#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "HashFunctionMora.h"

void diamond_structure(int d, HashFunctionMora& hf);
void generate_blocks(std::vector<uint8_t*>& res, unsigned long count, int block_length);
uint8_t* generate_rand_block(int len, std::vector<uint8_t*>& vec);
void fill_arr_rand(uint8_t* res, int len);
bool equal_array(uint8_t* a, uint8_t* b, int len);
void calculate_H(std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& res, std::vector<uint8_t*>& A, std::vector<uint8_t*>& M, HashFunctionMora& hf, uint8_t* N);
std::string arr_to_string(uint8_t* arr, int len);
void calculate_distinct_M1(std::vector<uint8_t*>& res, std::vector<uint8_t*>& already_exist, unsigned long count, int len);
uint8_t* generate_rand_distinct_block(std::vector<uint8_t*>& vec1, std::vector<uint8_t*>& vec2, int len);

void print_array(uint8_t* arr, int len)
{
	std::cout << "0x";
	for (int i = 0; i < len; i++)
	{
		printf("%02x", arr[i]);
	}
	std::cout << std::endl;
}

void add64(uint8_t* block)
{
	int internal = 0;

	for (int i = 7; i >= 0; i--)
	{
		if (i == 7)
		{
			internal = block[i] + 0x40;
		}
		else
		{
			internal = block[i] + (internal >> 8);
		}
		block[i] = internal & 0xff;
	}
}

int main()
{
	HashFunctionMora hf(8, 4);
	diamond_structure(4, hf);	

	/*uint8_t messagef[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	uint8_t message0[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t message02[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t message0_short[4] = { 0x00, 0x00, 0x00, 0x01 };
	uint8_t message_rand[8] = {0x01, 0xd4, 0x44, 0x90, 0x7e, 0xfb, 0x8c, 0xf7 };
	HashFunctionMora hash_func(8, 4);
	print_array(message_rand, 8);
	hash_func.gN(message0, message_rand, message0);
	hash_func.print_hash(message0);
	std::cout << std::endl;
	hash_func.gN(message0_short, message_rand, message02);
	hash_func.print_hash(message0_short);*/

	/*uint8_t a[4] = { 0xf4, 0x64, 0x63, 0x67 };
	std::string key = "";
	std::ostringstream convert;
	for (int i = 0; i < 4; i++) {
		convert << a[i];
	}
	key = convert.str();

	std::cout << key << std::endl;*/

	std::vector<uint8_t*> gg;
	/*generate_blocks(gg, 4, 8);
	for (auto a : gg)
	{
		print_array(a, 8);
	}*/

	uint8_t* g = new uint8_t[8];
	gg.push_back(g);
	for (auto& a : gg)
		if (a == g)
			std::cout << "ha" << std::endl;
}

void diamond_structure(int d, HashFunctionMora& hf)
{
	int n = hf.get_HASH_LEN();

	if (d <= 1 || d >= n * 8 / 2)
		return;

	unsigned long count = (unsigned long)pow(2, d);

	//��������� 2^d �����
	srand(time(nullptr));
	std::vector<uint8_t*> A;
	generate_blocks(A, count, n);

	// ���� �������:
	// tmp_A - ��������� A �� ��������� ���� (vector<uint8_t*>). ��������� ����� ������ (�������� ������).
	// B - ��������� ������ ��������, ���������� �������� (vector<vector<pair<std::string, std::string>>>).
	// M - ��������� ���������, ���������� ����������� ����� ��������� (vector<uint8_t*>). ��������� ����� ������ (���������� ������).
	// H - ��������� ����������� ������ ������� ������ �� ����� ������ �� A � �� ����� ����������� �� M (std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>),
	//	���: Key - ��� ��������� ������� ������ ���������� � ������. Value - ���� (���, ���������), � ������� ��������� ������� ������ � �������� Key.
	//	��� pair �� ���������� ����� ������, ������ ��� ������ ������� pair ������ �� A, ������ ������� �� pair ������ �� M.
	// M' - ���������� M, �� � M �� ������������.
	// H' - ���������� H, �� ���������� ��������� �� ��������� M'.

	std::vector<uint8_t*> tmp_A;
	std::vector<std::vector<std::pair<std::string, std::string>>> B;
	std::vector<uint8_t*> M;
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H;
	std::vector<uint8_t*> M1;
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H1;

	uint8_t* N = new uint8_t[n];
	std::fill_n(N, n, 0);

	for (size_t jump = d; jump >= 2; jump--)
	{
		std::cout << "jump: " << jump << std::endl;

		std::vector<std::pair<std::string, std::string>> B_in_jump;
		unsigned long m_cardinality = (unsigned long)pow(2, ((8 * n - jump) / 2) - 1);
		generate_blocks(M, m_cardinality, n);				std::cout << "card M: " << M.size() << std::endl;
		calculate_H(H, A, M, hf, N);						std::cout << "card H: " << H.size() << std::endl;

		for (size_t phase = jump; phase >= 2; phase--)
		{
			std::cout << " phase: " << phase << std::endl;

			size_t max_step = (size_t)pow(2, phase - 2);
			for (size_t step = 0; step < max_step; step++)
			{
				std::cout << "  step: " << step << std::endl;

				unsigned long m1_cardinality = (unsigned long)std::ceil(pow(2, (8 * n - phase) / 2 + 1) / (pow(2, phase) - 2 * step));
				calculate_distinct_M1(M1, M, m1_cardinality, n);				std::cout << "card M1: " << M1.size() << std::endl;
				calculate_H(H1, A, M1, hf, N);									std::cout << "card H1: " << H1.size() << std::endl;

				for (auto h : H)
				{
					std::string hash_res = h.first;
					std::pair<uint8_t*, uint8_t*> collision;
					try {
						collision = H1.at(hash_res);

						// � tmp_A �������� hash_res.to_arr (�������� ������)
						// � B_in_jump �������� h.second, collision
						// ������� �� A h.second.first, collision.first (���������� ��� ������)
						// � M �������� M1
						// M1 ����������
						// H ���������� 
						// H ��������� � ������ A � M
						// H1 ����������
					}
					catch (std::out_of_range ex) {
						continue;
					}
				}
			}
		}
	}
}

void generate_blocks(std::vector <uint8_t*>& res, unsigned long count, int block_length)
{
	for (size_t i = 0; i < count; i++)
	{
		uint8_t* block = generate_rand_block(block_length, res);
		res.push_back(block);
	}
}

uint8_t* generate_rand_block(int len, std::vector<uint8_t*>& vec)
{
	uint8_t* block = new uint8_t[len];
	while (true)
	{
		fill_arr_rand(block, len);
		bool correct_block = true;
		for (auto arr : vec)
		{
			correct_block &= !equal_array(arr, block, len);
			if (!correct_block)
			{
				break;
			}
		}
		if (correct_block)
		{
			return block;
		}
	}
}

void fill_arr_rand(uint8_t* res, int len)
{
	for (int i = 0; i < len; i++)
	{
		res[i] = rand() % UINT8_MAX;
	}
}

bool equal_array(uint8_t* a, uint8_t* b, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}

void calculate_H(std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& res, std::vector<uint8_t*>& A, std::vector<uint8_t*>& M, HashFunctionMora& hf, uint8_t* N)
{
	int n = hf.get_HASH_LEN();
	for (auto hash : A)
	{
		for (auto m : M)
		{
			uint8_t* res_hash = new uint8_t[n];
			memcpy(res_hash, hash, n);
			hf.gN(res_hash, m, N);
			res.insert(std::make_pair(arr_to_string(res_hash, n), std::make_pair(hash, m)));
			delete[] res_hash;
		}
	}
}

std::string arr_to_string(uint8_t* arr, int len)
{
	std::ostringstream convert;
	for (int i = 0; i < len; i++) {
		convert << arr[i];
	}
	return convert.str();
}

void calculate_distinct_M1(std::vector<uint8_t*>& res, std::vector<uint8_t*>& already_exist, unsigned long count, int len)
{
	for (size_t i = 0; i < count; i++)
	{
		uint8_t* block = generate_rand_distinct_block(res, already_exist, len);
		res.push_back(block);
	}
}

uint8_t* generate_rand_distinct_block(std::vector<uint8_t*>& vec1, std::vector<uint8_t*>& vec2, int len)
{
	uint8_t* block = new uint8_t[len];
	
	while (true)
	{
		fill_arr_rand(block, len);
		bool correct_block = true;
		for (auto arr : vec1)
		{
			correct_block &= !equal_array(arr, block, len);
			if (!correct_block)
			{
				break;
			}
		}
		for (auto arr : vec2)
		{
			correct_block &= !equal_array(arr, block, len);
			if (!correct_block)
			{
				break;
			}
		}
		if (correct_block)
		{
			return block;
		}
	}
}