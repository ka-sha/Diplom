#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "HashFunctionMora.h"

void diamond_structure(int d, HashFunctionMora& hf);
void generate_blocks(std::vector<uint8_t*>& res, size_t count, int block_length);
void generate_blocks(std::vector<uint8_t*>& res, size_t init_count, size_t count, int block_length);
uint8_t* generate_rand_block(int len, std::vector<uint8_t*>& vec);
void fill_arr_rand(uint8_t* res, int len);
bool equal_array(uint8_t* a, uint8_t* b, int len);
void calculate_H(std::unordered_map<std::string, std::pair<uint8_t*,
	uint8_t*>>& res,
	std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	HashFunctionMora& hf,
	uint8_t* N);
std::string arr_to_string(uint8_t* arr, int len);
uint8_t* string_to_arr(std::string str);
void calculate_distinct_M1(std::vector<uint8_t*>& res, std::vector<uint8_t*>& already_exist, size_t count, int len);
uint8_t* generate_rand_distinct_block(std::vector<uint8_t*>& vec1, std::vector<uint8_t*>& vec2, int len);
void process_last_phase(std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	uint8_t* N);
void add64(uint8_t* block);
void write_result_to_file(std::vector<std::vector<std::pair<std::string, std::string>>>& B);
std::string arr_to_hexstring(uint8_t* arr, size_t len);


void print_array(uint8_t* arr, int len)
{
	std::cout << "0x";
	for (int i = 0; i < len; i++)
	{
		printf("%02x", arr[i]);
	}
	std::cout << std::endl;
}

int main()
{
	HashFunctionMora hf(8, 4);
	diamond_structure(3, hf);	

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

	/*std::vector<uint8_t*> gg;
	generate_blocks(gg, 4, 8);
	for (auto a : gg)
	{
		print_array(a, 8);
	}

	uint8_t* g = new uint8_t[8];
	gg.push_back(g);
	for (auto& a : gg)
		if (a == g)
			std::cout << "ha" << std::endl;*/

	/*uint8_t* a = new uint8_t[4];
	for (int i = 0; i < 4; i++) {
		a[i] = i;
		std::cout << (int)a[i] << std::endl;
	}
	std::cout << std::endl;
	std::vector<uint8_t*> vec;
	vec.push_back(a);
	for (int i = 0; i < 4; i++)
		std::cout << (int)vec.at(0)[i] << std::endl;
	std::cout << std::endl;
	uint8_t* b = vec.at(0);
	delete[] b;
	for (int i = 0; i < 4; i++)
		std::cout << (int)vec.at(0)[i] << std::endl;
	std::cout << std::endl;
	vec.clear();
	for (int i = 0; i < 4; i++)
		std::cout << (int)a[i] << std::endl;*/
	
	/*std::vector<std::vector<int>> vec1;
	std::vector<int> vec2;
	vec2.push_back(4);
	vec2.push_back(6);

	vec1.push_back(vec2);
	std::cout << vec1.at(0).at(0) << ", " << vec1.at(0).at(1) << std::endl;
	vec2.clear();
	std::cout << vec1.at(0).at(0) << ", " << vec1.at(0).at(1) << std::endl;*/


	/*time_t t = std::time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &t);
	
	std::stringstream textStream;
	textStream << std::put_time(&timeinfo, "%d-%m-%Y-%H-%M-%S.txt");
	std::cout << textStream.str() << std::endl;*/
}

void diamond_structure(int d, HashFunctionMora& hf)
{
	int n = hf.get_HASH_LEN();

	if (d <= 1 || d >= n * 8 / 2)
		return;

	unsigned long count = (unsigned long)pow(2, d);

	//генерация 2^d хэшей
	srand(time(nullptr));
	std::vector<uint8_t*> A;
	generate_blocks(A, count, n);

	// надо хранить:
	// tmp_A - множество A на следующем шаге (vector<uint8_t*>). Создаются новые ссылки (выделяем память).
	// B - множество парных значений, образующих коллизию (vector<vector<pair<std::string, std::string>>>).
	// M - множество сообщений, содержащее определённое число элементов (vector<uint8_t*>). Создаются новые ссылки (выделяется память).
	// H - множество результатов работы функции сжатия со всеми хэшами их A и со всеми сообщениями из M (std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>),
	//	где: Key - это результат функции сжатия приведённый к строке. Value - пара (хэш, сообщение), к которым применили функцию сжатия и получили Key.
	//	Для pair не выделяется новая память, потому что первый элемент pair берётся из A, второй элемент из pair берётся из M.
	// M' - аналогично M, но с M не пересекается.
	// H' - аналогично H, но использует сообщения из множества M'.

	std::vector<uint8_t*> tmp_A;
	std::vector<std::vector<std::pair<std::string, std::string>>> B;
	std::vector<uint8_t*> M;
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H;
	std::vector<uint8_t*> M1;
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H1;
	std::vector<std::pair<std::string, std::string>> B_in_jump;

	uint8_t* N = new uint8_t[n];
	std::fill_n(N, n, 0);

	for (size_t jump = d; jump >= 2; jump--)
	{
		std::cout << "jump: " << jump << std::endl;

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

				for (auto& h : H)
				{
					std::string hash_res = h.first;
					std::pair<uint8_t*, uint8_t*> collision;
					try {
						collision = H1.at(hash_res);
						// в tmp_A положить hash_res.to_arr (выделяем память)
						// в B_in_jump положить h.second, collision
						// удалить из A h.second.first, collision.first (освободить эту память)
						// к M добавить M1
						// M1 освободить
						// H освободить 
						// H заполнить с новыми A и M
						// H1 освободить
						
						tmp_A.push_back(string_to_arr(hash_res));
						uint8_t* h1 = h.second.first;
						uint8_t* h2 = collision.first;
						B_in_jump.push_back(std::make_pair(arr_to_string(h1, n), arr_to_string(h.second.second, 8)));
						B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(collision.second, 8)));
						A.erase(std::remove(A.begin(), A.end(), h1), A.end());
						delete[] h1;
						A.erase(std::remove(A.begin(), A.end(), h2), A.end());
						delete[] h2;
						M.insert(M.end(), M1.begin(), M1.end());
						M1.clear();
						H1.clear();
						H.clear();
						if (phase != 2)
						{
							calculate_H(H, A, M, hf, N);
						}

						break;
					}
					catch (std::out_of_range ex) {
						continue;
					}
				}
			}
		}

		// обработка последней фазы
		generate_blocks(M, M.size(), (size_t)pow(2, (8 * n) / 2), n);
		process_last_phase(A, M, H, B_in_jump, tmp_A, hf, N); // A пустое после этого // B_in_jump добавляется последняя коллизия

		// M освободить (с очисткой памяти всех массивов)
		// H освободить
		H.clear();
		for (auto& m : M)
		{
			delete[] m;
		}
		M.clear();

		// B_in_jump добавить в B
		// B_in_jump.clear
		// В A добавить tmp_A. Затем tmp_A.clear();
		// N прибавить 64
		B.push_back(B_in_jump);
		B_in_jump.clear();
		for (auto& h : tmp_A)
		{
			A.push_back(h);
		}
		tmp_A.clear();
		add64(N);
	}

	// обработка последнего джампа
	generate_blocks(M, (size_t)pow(2, (8 * n) / 2), n);
	process_last_phase(A, M, H, B_in_jump, tmp_A, hf, N);
	B.push_back(B_in_jump);
	write_result_to_file(B);// всё

	for (auto& h : tmp_A)
	{
		delete[] h;
	}
	for (auto& m : M)
	{
		delete[] m;
	}

	tmp_A.clear();
	B.clear();
	M.clear();
	H.clear();
	B_in_jump.clear();
	delete[] N;
}

void generate_blocks(std::vector<uint8_t*>& res, size_t count, int block_length)
{
	generate_blocks(res, 0, count, block_length);
}

void generate_blocks(std::vector<uint8_t*>& res, size_t init_count, size_t count, int block_length)
{
	for (size_t i = init_count; i < count; i++)
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
		for (auto& arr : vec)
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
	for (auto& hash : A)
	{
		for (auto& m : M)
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

uint8_t* string_to_arr(std::string str)
{
	size_t len = str.length();
	uint8_t* res = new uint8_t[len];

	for (size_t i = 0; i < len; i++)
	{
		res[i] = (uint8_t)str.at(i);
	}
	return res;
}

void calculate_distinct_M1(std::vector<uint8_t*>& res, std::vector<uint8_t*>& already_exist, size_t count, int len)
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
		for (auto& arr : vec1)
		{
			correct_block &= !equal_array(arr, block, len);
			if (!correct_block)
			{
				break;
			}
		}
		for (auto& arr : vec2)
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

void process_last_phase(std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	uint8_t* N)
{
	int n = hf.get_HASH_LEN();
	std::vector<uint8_t*> A1;
	A1.push_back(A.at(0));
	
	calculate_H(H, A1, M, hf, N);
	uint8_t* h2 = A.at(1);
	for (auto& m : M)
	{
		uint8_t* hash = new uint8_t[n];
		memcpy(hash, h2, n);
		hf.gN(hash, m, N);

		std::pair<uint8_t*, uint8_t*> collision;
		try {
			collision = H.at(arr_to_string(hash, n));

			tmp_A.push_back(hash);
			B_in_jump.push_back(std::make_pair(arr_to_string(collision.first, n), arr_to_string(collision.second, 8)));
			B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(m, 8)));
			delete[] h2;
			delete[] A1.at(0);
			A1.clear();

			break;
		}
		catch(std::out_of_range ex) {
			continue;
		}
	}
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

void write_result_to_file(std::vector<std::vector<std::pair<std::string, std::string>>>& B)
{
	time_t t = std::time(nullptr);
	struct tm time_info;
	localtime_s(&time_info, &t);
	std::stringstream text_stream;
	text_stream << std::put_time(&time_info, "%d-%m-%Y-%H-%M-%S.txt");

	std::ofstream file;
	file.open(text_stream.str());
	size_t jump = B.size();
	for (auto& vec : B)
	{
		file << "jump №" << jump << ":" << std::endl << std::endl;
		int num = 1;
		for (auto& pair : vec)
		{
			auto h = pair.first;
			auto m = pair.second;
			file << "(h" << num << ", x" << num << "): (" << arr_to_hexstring(string_to_arr(h), h.length()) << ", " << arr_to_hexstring(string_to_arr(m), m.length()) << ")" << std::endl;
			num++;
		}
		jump--;
	}
	
	file.close();
}

std::string arr_to_hexstring(uint8_t* arr, size_t len)
{
	std::stringstream ss;
	ss << std::hex;

	for (int i = 0; i < len; i++)
	{
		ss << std::setw(2) << std::setfill('0') << (int)arr[i];
	}

	return ss.str();
}