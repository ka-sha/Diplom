#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "HashFunctionMora.h"

bool process_args(int* d, int* n, const int argc, char** argv);
void diamond_structure(const int d, HashFunctionMora& hf);
void generate_blocks(std::vector<uint8_t*>& res, const size_t count, const int block_length, const bool distinct);
void generate_blocks(std::vector<uint8_t*>& res, const size_t init_count, const size_t count, const int block_length, const bool distinct);
uint8_t* generate_rand_block(const std::vector<uint8_t*>& vec, const int len, const bool distinct);
void fill_arr_rand(uint8_t* res, const int len);
bool equal_array(const uint8_t* a, const uint8_t* b, const int len);
void calculate_H(std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& res,
	const std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M,
	HashFunctionMora& hf,
	const uint8_t* N);
std::string arr_to_string(const uint8_t* arr, const int len);
uint8_t* string_to_arr(const std::string str);
void clear_vec(std::vector<uint8_t*>& vec);
void process_last_two_hashes_in_jump(std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN);
void process_last_two_hashes(std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN);
void add64(uint8_t* block);
void write_result_to_file(const std::vector<std::vector<std::pair<std::string, std::string>>>& B);
std::string arr_to_hexstring(const uint8_t* arr, const size_t len);
void log(const std::string message);


int main(int argc, char** argv)
{
	int n = -1;
	int d = -1;
	
	if (!process_args(&d, &n, argc, argv))
	{
		return -1;
	}

	HashFunctionMora hf(8, n);

	log("Start diamond structure construction with d = " + std::to_string(d) + ", and n = " + std::to_string(n));
	diamond_structure(d, hf);	
}

bool process_args(int* d, int* n, const int argc, char** argv)
{
	const std::string info_message = "Incorrect input. Need somethig like this:\nsecond_try.exe -d <integer> [-n <integer>]";

	if ((argc != 3) && (argc != 5)) {
		log(info_message);
		return false;
	}
	if (argv[1][0] != '-' || argv[1][1] != 'd' || argv[1][2] != '\0')
	{
		log(info_message);
		return false;
	}
	*d = atoi(argv[2]);
	if (argc == 5)
	{
		if (argv[3][0] != '-' || argv[3][1] != 'n' || argv[3][2] != '\0')
		{
			log(info_message);
			return false;
		}
		*n = atoi(argv[4]);
	}
	else
	{
		*n = 8;
	}

	if (*n <= 0 || *n > 8)
	{
		log(info_message);
		return false;
	}
	return true;
}

void diamond_structure(const int d, HashFunctionMora& hf)
{
	const int n = hf.get_HASH_LEN();

	if (d <= 1 || d >= n * 8 / 2)
	{
		log("Incorrect input d = " + std::to_string(d) + ". d must be between 2 and " + std::to_string(n * 8 / 2));
		return;
	}

	// Изначальное множество хэшей(мощность 2 ^ d).Выделяется новая память
	std::vector<uint8_t*> A;
	// Множество A на следующем шаге. Выделяется новая память
	std::vector<uint8_t*> tmp_A;
	// Множество парных значений образующих коллизии. Первый вектор - все коллизии в джампе, вектор в векторе - пары (h, x), образующие коллизию.
	// Применяя функцию сжатия к элементам с индексвми 0 и 1, 2 и 3, 4 и 5 ... можно получить коллизию
	std::vector<std::vector<std::pair<std::string, std::string>>> B;
	// Генерируемое множество сообщений, среди которых ищются коллизии. Выделяется новая память
	std::vector<uint8_t*> M;
	// Множество результатов работы функции сжатия со всеми хэшами их A и со всеми сообщениями из M,
	// где: key - это результат функции сжатия приведённый к строке, value - пара (h,x), к которым применили функцию сжатия и получили key
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H;
	// Аналогично M, но не пересекается с M. Выделяется память
	std::vector<uint8_t*> M1;
	// Аналогично H, но используются сообщения из множества M1
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>> H1;
	// Пары (h, x), образующие коллизию, в пределах одного джампа
	std::vector<std::pair<std::string, std::string>> B_in_jump;

	const unsigned long init_hash_count = (unsigned long)pow(2, d);
	srand((unsigned int)time(nullptr));
	generate_blocks(A, init_hash_count, n, true);

	const int MESSAGE_LEN = 8;
	uint8_t* N = new uint8_t[MESSAGE_LEN];
	std::fill_n(N, MESSAGE_LEN, 0);
	bool collision_found = false;

	for (size_t jump = d; jump >= 2; jump--)
	{
		log("jump: " + std::to_string(jump));

		unsigned long m_cardinality = (unsigned long)pow(2, ((8 * n - jump) / 2) - 1);
		generate_blocks(M, m_cardinality, MESSAGE_LEN, false);
		log("cardinality of M: " + std::to_string(M.size()));
		calculate_H(H, A, M, hf, N);	
		log("cardinality of H: " + std::to_string(H.size()));

		for (size_t phase = jump; phase >= 2; phase--)
		{
			log(" phase: " + std::to_string(phase));

			const size_t max_step = (size_t)pow(2, phase - 2);
			for (size_t step = 0; step < max_step; step++)
			{
				log("  step: " + std::to_string(step));

				const unsigned long m1_cardinality = (unsigned long)std::ceil(pow(2, (8 * n - phase) / 2 + 1) / (pow(2, phase) - 2 * step));
				generate_blocks(M1, m1_cardinality, MESSAGE_LEN, false);
				log("   card M1: " + std::to_string(M1.size()));
				calculate_H(H1, A, M1, hf, N);								
				log("   card H1: " + std::to_string(H1.size()));

				for (auto& h : H)
				{
					std::string hash_res = h.first;
					try {
						std::pair<uint8_t*, uint8_t*> collision = H1.at(hash_res); // Есть элемент - переходим к след строке, нет элемента - попадаем в catch блок
						uint8_t* h1 = h.second.first;
						uint8_t* h2 = collision.first;
						uint8_t* m1 = h.second.second;
						uint8_t* m2 = collision.second;
						if (equal_array(h1, h2, n) || equal_array(m1, m2, MESSAGE_LEN))
						{
							continue;
						}

						log("    collision found");
						collision_found = true;

						tmp_A.push_back(string_to_arr(hash_res));
						B_in_jump.push_back(std::make_pair(arr_to_string(h1, n), arr_to_string(m1, MESSAGE_LEN)));
						B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(m2, MESSAGE_LEN)));

						log("    h1: " + arr_to_hexstring(h1, n) + ", m1: " + arr_to_hexstring(m1, MESSAGE_LEN));
						log("    h2: " + arr_to_hexstring(h2, n) + ", m2: " + arr_to_hexstring(m2, MESSAGE_LEN));

						A.erase(std::remove(A.begin(), A.end(), h1), A.end());
						delete[] h1;
						A.erase(std::remove(A.begin(), A.end(), h2), A.end());
						delete[] h2;
						log("    cardinality of A: " + std::to_string(A.size()));

						M.insert(M.end(), M1.begin(), M1.end());
						log("    cardinality of M: " + std::to_string(M.size()));
						M1.clear();
						H1.clear();
						H.clear();
						if (phase != 2)
						{
							calculate_H(H, A, M, hf, N);
							log("    cardinality of H: " + std::to_string(H.size()));
						}

						break;
					}
					catch (std::out_of_range ex) {
						continue;
					}
				}
				if (!collision_found)
				{
					step--; // Остаёмся на том же шаге, если коллизия не найдена
					H1.clear();
					clear_vec(M1);
				}
				collision_found = false;
			}
		}

		log(" phase: 1");
		process_last_two_hashes_in_jump(A, M, H, B_in_jump, tmp_A, hf, N, MESSAGE_LEN);

		clear_vec(M);
		H.clear();

		B.push_back(B_in_jump);
		B_in_jump.clear();
		for (auto& h : tmp_A)
		{
			A.push_back(h);
		}
		tmp_A.clear();
		add64(N);
	}

	log("jump: 1");
	process_last_two_hashes_in_jump(A, M, H, B_in_jump, tmp_A, hf, N, MESSAGE_LEN);
	B.push_back(B_in_jump);
	write_result_to_file(B);

	clear_vec(M);
	clear_vec(tmp_A);
}

void generate_blocks(std::vector<uint8_t*>& res, const size_t count, const int block_length, const bool distinct)
{
	generate_blocks(res, 0, count, block_length, distinct);
}

void generate_blocks(std::vector<uint8_t*>& res, const size_t init_count, const size_t count, const int block_length, const bool distinct)
{
	for (size_t i = init_count; i < count; i++)
	{
		uint8_t* block = generate_rand_block(res, block_length, distinct);
		res.push_back(block);
	}
}

uint8_t* generate_rand_block(const std::vector<uint8_t*>& vec, const int len, const bool distinct)
{
	uint8_t* block = new uint8_t[len];
	do
	{
		fill_arr_rand(block, len);
		if (!distinct)
		{
			return block;
		}
		bool correct_block = true;
		for (const auto& arr : vec)
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
	} while (true);
}

void fill_arr_rand(uint8_t* res, const int len)
{
	for (int i = 0; i < len; i++)
	{
		res[i] = rand() % UINT8_MAX;
	}
}

bool equal_array(const uint8_t* a, const uint8_t* b, const int len)
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

void calculate_H(std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& res,
	const std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M,
	HashFunctionMora& hf,
	const uint8_t* N)
{
	const int n = hf.get_HASH_LEN();
	for (const auto& hash : A)
	{
		for (const auto& m : M)
		{
			uint8_t* res_hash = new uint8_t[n];
			memcpy(res_hash, hash, n);
			hf.gN(res_hash, m, N);
			res.insert(std::make_pair(arr_to_string(res_hash, n), std::make_pair(hash, m)));
			delete[] res_hash;
		}
	}
}

std::string arr_to_string(const uint8_t* arr, const int len)
{
	std::ostringstream convert;
	for (int i = 0; i < len; i++) {
		convert << arr[i];
	}
	return convert.str();
}

uint8_t* string_to_arr(const std::string str)
{
	const size_t len = str.length();
	uint8_t* res = new uint8_t[len];

	for (size_t i = 0; i < len; i++)
	{
		res[i] = (uint8_t)str.at(i);
	}
	return res;
}

void clear_vec(std::vector<uint8_t*>& vec)
{
	for (auto& arr : vec)
	{
		delete[] arr;
	}
	vec.clear();
}

void process_last_two_hashes_in_jump(std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN)
{
	const int n = hf.get_HASH_LEN();

	while (A.size())
	{
		generate_blocks(M, M.size(), (size_t)pow(2, (8 * n) / 2), MESSAGE_LEN, false);
		log("  cardinality of M: " + std::to_string(M.size()));
		process_last_two_hashes(A, M, H, B_in_jump, tmp_A, hf, N, MESSAGE_LEN); // После выполнения, при нахождении коллизии, A - пустое,
																		   // В B_in_jump добавляется последняя коллизия в джампе
		if (A.size())
		{
			clear_vec(M);
			H.clear();
		}
	}
}

void process_last_two_hashes(std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M,
	std::unordered_map<std::string, std::pair<uint8_t*, uint8_t*>>& H,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN)
{
	const int n = hf.get_HASH_LEN();
	std::vector<uint8_t*> A1;
	A1.push_back(A.at(0));
	
	calculate_H(H, A1, M, hf, N);
	uint8_t* h2 = A.at(1);
	for (const auto& m : M)
	{
		uint8_t* hash = new uint8_t[n];
		memcpy(hash, h2, n);
		hf.gN(hash, m, N);

		try {
			std::pair<uint8_t*, uint8_t*> collision = H.at(arr_to_string(hash, n));
			uint8_t* h1 = collision.first;
			if (equal_array(h1, h2, n))
			{
				continue;
			}

			log("    collision found");

			tmp_A.push_back(hash);
			B_in_jump.push_back(std::make_pair(arr_to_string(h1, n), arr_to_string(collision.second, MESSAGE_LEN)));
			B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(m, 8)));

			log("   h1: " + arr_to_hexstring(collision.first, n) + ", m1: " + arr_to_hexstring(collision.second, 8));
			log("   h2: " + arr_to_hexstring(h2, n) + ", m2: " + arr_to_hexstring(m, 8));

			delete[] h2;
			A.clear();
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

void write_result_to_file(const std::vector<std::vector<std::pair<std::string, std::string>>>& B)
{
	time_t t = std::time(nullptr);
	struct tm time_info;
	localtime_s(&time_info, &t);
	std::stringstream text_stream;
	text_stream << std::put_time(&time_info, "%d-%m-%Y-%H-%M-%S_collision.txt");

	std::ofstream file;
	file.open(text_stream.str());
	size_t jump = B.size();
	for (const auto& vec : B)
	{
		file << "jump №" << jump << ":" << std::endl;
		int num = 1;
		for (const auto& pair : vec)
		{
			auto h = pair.first;
			auto m = pair.second;
			file << "(h" << num << ", x" << num << "): (" << arr_to_hexstring(string_to_arr(h), h.length()) << ", "
				<< arr_to_hexstring(string_to_arr(m), m.length()) << ")" << std::endl;
			num++;
		}
		file << std::endl;
		jump--;
	}
	
	file.close();
}

std::string arr_to_hexstring(const uint8_t* arr, const size_t len)
{
	std::stringstream ss;
	ss << std::hex;

	for (int i = 0; i < len; i++)
	{
		ss << std::setw(2) << std::setfill('0') << (int)arr[i];
	}

	return ss.str();
}

void log(const std::string message)
{
	std::cout << message << std::endl;
}