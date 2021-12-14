#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include "HashFunctionMora.h"
#include <fstream>

constexpr auto n = 64;
constexpr auto LEN = 8;

using namespace std::chrono;


void X(const uint8_t* a, const uint8_t* b, uint8_t* res);

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

static const uint16_t T[4][16] = {
	{ 0x8147, 0x643f, 0xc86d, 0x2d15, 0x328e, 0xfae3, 0xb3c9, 0xac52,
	  0xe578, 0x9edc, 0x492a, 0x7ba4, 0x0000, 0xd7f6, 0x56b1, 0x1f9b },
	{ 0x4564, 0x37b3, 0x6e56, 0x1c81, 0x8ac8, 0xe49e, 0xcfac, 0x59e5,
	  0x72d7, 0xd32d, 0x2b32, 0xa1fa, 0x0000, 0xf81f, 0xbd7b, 0x9649 },
	{ 0xefd2, 0xd918, 0x9123, 0xa7e9, 0xfd94, 0x6cb7, 0x1246, 0x483b,
	  0x36ca, 0xb5af, 0x7ef1, 0x8365, 0x0000, 0xcb5e, 0x248c, 0x5a7d },
	{ 0xd877, 0x16ff, 0x2cdd, 0xe255, 0x93ee, 0xbf33, 0x4b99, 0x3a22,
	  0xce88, 0xa9cc, 0xf4aa, 0x6744, 0x0000, 0x5d66, 0x8511, 0x71bb }
};




void fast_LPS(uint8_t* res)
{
	uint16_t* tmp_res = new uint16_t[4];
	tmp_res[0] = T[0][res[0] >> 4] ^ T[1][res[2] >> 4] ^ T[2][res[4] >> 4] ^ T[3][res[6] >> 4];
	tmp_res[1] = T[0][res[0] & 0xf] ^ T[1][res[2] & 0xf] ^ T[2][res[4] & 0xf] ^ T[3][res[6] & 0xf];
	tmp_res[2] = T[0][res[1] >> 4] ^ T[1][res[3] >> 4] ^ T[2][res[5] >> 4] ^ T[3][res[7] >> 4];
	tmp_res[3] = T[0][res[1] & 0xf] ^ T[1][res[3] & 0xf] ^ T[2][res[5] & 0xf] ^ T[3][res[7] & 0xf];

	memcpy(res, tmp_res, 8);
	delete[] tmp_res;
}

void fast_get_key(uint8_t* K, int i)
{
	X(K, C[i], K);
	fast_LPS(K);
}

void fast_E(uint8_t* K, uint8_t* m, uint8_t* res)
{
	X(K, m, res);

	for (int i = 0; i < 8; i++)
	{
		fast_LPS(res);
		fast_get_key(K, i);
		X(K, res, res);
	}
}

void fast_gN(uint8_t* h, uint8_t* m, uint8_t* N)
{
	uint8_t* K = new uint8_t[8];
	uint8_t* tmp_res = new uint8_t[8];

	X(h, N, K);
	fast_LPS(K);

	fast_E(K, m, tmp_res);

	X(tmp_res, h, tmp_res);
	X(tmp_res, m, h);

	delete[] K;
	delete[] tmp_res;
}




void L(uint8_t* block)
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

void P(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];
	std::fill_n(res, 8, 0);

	for (int i = 0; i < 16; i++) {
		const uint8_t index = TAU[i];
		const uint8_t block_index = index / 2;
		const uint8_t block_value = (index % 2 == 0) ? block[block_index] >> 4 : block[block_index] & 0xf;
		res[i / 2] ^= (i % 2 == 0) ? (block_value << 4) : block_value;
	}
	memcpy(block, res, 8);
	delete[] res;
}

void S(uint8_t* block)
{
	uint8_t* res = new uint8_t[8];

	for (int i = 0; i < 8; i++)
	{
		const uint8_t left_state = block[i] >> 4;
		const uint8_t right_state = block[i] & 0xf;
		res[i] = (PI[left_state] << 4) ^ PI[right_state];
	}
	memcpy(block, res, 8);
	delete[] res;
}

void X(const uint8_t* a, const uint8_t* b, uint8_t* res)
{
	for (int i = 0; i < 8; i++)
	{
		res[i] = a[i] ^ b[i];
	}
}

void get_key(uint8_t* K, int i)
{
	X(K, C[i], K);
	S(K);
	P(K);
	L(K);
}

void E(uint8_t* K, uint8_t* m, uint8_t* res)
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

void gN(uint8_t* h, uint8_t* m, uint8_t* N)
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


void print_array(uint8_t* arr)
{
	for (int i = 0; i < LEN; i++)
	{
		std::cout << std::hex << std::setw(2) << (int)arr[i] << " ";
	}
	std::cout << std::endl;
}

void convert_hex(uint8_t* dest, size_t count, const char* src)
{
	char buf[3];
	size_t i;
	int value;
	for (i = 0; i < count && *src; i++) {
		buf[0] = *src++;
		buf[1] = '\0';
		if (*src) {
			buf[1] = *src++;
			buf[2] = '\0';
		}
		if (sscanf_s(buf, "%x", &value) != 1)
			break;
		dest[i] = value;
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

bool equal_array(uint8_t* a, uint8_t* b)
{
	for (int i = 0; i < LEN; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}

unsigned long long int count_bound(unsigned long long int begin, unsigned long long int end)
{
	if (begin <= end)
	{
		return end - begin;
	}
	else
	{
		return ULLONG_MAX + (end - begin);
	}
}

void number2array(unsigned long long int number, uint8_t* arr_result)
{
	for (int i = 0; i < LEN; i++)
	{
		arr_result[i] = (int)((number >> 8*(7 - i)) & 0xff);
	}
}

void add1(uint8_t* block)
{
	for (int i = 7; i >= 0; i--)
	{
		if (block[i] != UINT8_MAX)
		{
			block[i] = (uint8_t)(block[i] + 1);
			break;
		}
		else
		{
			block[i] = 0;
		}
	}
}

std::vector<uint8_t*> generate_blocks(int count)
{
	std::vector<uint8_t*> result;
	uint8_t* block = new uint8_t[LEN];
	std::fill_n(block, LEN, 0);

	for (int i = 0; i < count; i++)
	{
		result.push_back(block);
		add1(block);
	}
	return result;
}

void diamond_structure(int d)
{
	if (d <= 1 || d >= 32)
		return;

	long count = (long)pow(2, d);
	//auto start = high_resolution_clock::now();
	
	//генерация 2^d хэшей
	std::vector<uint8_t*> A = generate_blocks(count);

	/*auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << " microseconds" << endl;*/

	//for (int i = 0; i < count; i++) //проверяю, что нагенерил. Надо удалить отсюда
	//{
	//	for (int j = 0; j < 8; j++)
	//	{
	//		cout << (int)init_H.at(i)[j] << " ";
	//	}
	//	cout << endl;
	//} // до сюда
	 
	//HashFunctionMora hf(8);
	uint8_t* N = new uint8_t[8];
	std::fill_n(N, 8, 0);

	std::vector<uint8_t*> temp_A;
	std::vector<std::vector<std::pair<uint8_t*, uint8_t*>>> B; //по индексу первого вектора опрделяется номер джампа, по индексу второго вектора определяется пара (h, x), имеющая коллизию на подходящем джампе
	std::vector<std::pair<uint8_t*, uint8_t*>> B_in_jump;
	uint8_t* message = new uint8_t[LEN]; //сообщение из множества M. прибавляя к message едиичку перебираем все сообщения из M.
	unsigned long long int message_first; //первый элемет из M с него начинаем перебирать
	unsigned long long int message_last; //последний элемент из M, чем заканчиваем перебирать, сам этот элемент не считаем. То есть M состоит из элементов от 0 до (message_last - 1)

	uint8_t* _1message = new uint8_t[LEN]; //сообщение из множества M'. прибавляя к message едиичку перебираем все сообщения из M'.
	unsigned long long int _1message_first;//первый элемет из M' с него начинаем перебирать
	unsigned long long int _1message_last; //последний элемент из M', чем заканчиваем перебирать, сам этот элемент не считаем.
	//M и H задать не получится. Слишком много памяти требуется

	uint8_t* hi = new uint8_t[LEN];
	uint8_t* h1 = new uint8_t[LEN];
	uint8_t* hk = new uint8_t[LEN];
	uint8_t* h2 = new uint8_t[LEN];

	for (int jump = d; jump >= 2; jump--) //пошли jumps с d по 2
	{
		std::cout << "jump " << jump << std::endl;
		
		//TODO: init // вроде как сделано
		for (int phase = jump; phase >= 2; phase--) //пошли фазы с d по 2
		{
			std::cout << " phase " << phase << std::endl;

			for (int step = 0; step < pow(2, phase - 2); step++) //шаги пошли. шаги от 0 до 2^(j-2) - 1 (включительно). j - это номер фазы
			{
				std::cout << "  step " << step << std::endl;

				message_first = 0;
				message_last = message_first + (unsigned long long int)ceil((pow(2, (n+phase)/2 - 1)) / (pow(2, phase) - 2*step));
				number2array(message_first, message);

				_1message_first = message_last;
				_1message_last = _1message_first + (unsigned long long int)ceil((pow(2, (n-phase)/2 + 1)) / (pow(2, phase) - 2*step));
				number2array(_1message_first, _1message);
				
				std::cout << "   Set M from " << message_first << " to " << message_last << ". Cardinality: " << count_bound(message_first, message_last) << std::endl;
				std::cout << "   Set M' from " << _1message_first << " to " << _1message_last << ". Cardinality: " << count_bound(_1message_first, _1message_last) << std::endl;

				bool total_break = false;
				for (size_t i = 0; i < A.size(); i++) //перебираем все хэши из A
				{
					hi = A.at(i);
					std::cout << "i = " << i << std::endl;
					for (size_t j = 0; j < count_bound(message_first, message_last); j++)
					{
						memcpy(h1, hi, LEN);
						gN(h1, message, N);
						for (size_t k = i + 1; k < A.size(); k++)
						{
							hk = A.at(k);
							std::cout << "k = " << k << std::endl;
							for (size_t t = 0; t < count_bound(_1message_first, _1message_last); t++)
							{
								memcpy(h2, hk, LEN);
								gN(h2, _1message, N);
								if (equal_array(h1, h2))
								{
									A.erase(A.begin() + i); //убираем hi
									A.erase(A.begin() + (k - 1)); //убираем hk. -1 потому что из-за предвдущего шага в А меньше элеметов, и все оставшиеся сместились влево на 1
									temp_A.push_back(h1); //добавили хэш для следующего джампа
									std::pair<uint8_t*, uint8_t*> h1_and_x1(hi, message);
									std::pair<uint8_t*, uint8_t*> h2_and_x2(hk, _1message);
									B_in_jump.push_back(h1_and_x1);
									B_in_jump.push_back(h2_and_x2);
									total_break = true;

									std::cout << "   colliding pairs hi: ";
									print_array(hi);
									std::cout << "   and message: ";
									print_array(message);
									std::cout << "   colliding pairs hk: ";
									print_array(hk);
									std::cout << "   and message: ";
									print_array(_1message);
									std::cout << "   result: ";
									print_array(h1);
									std::cout << std::endl;

									break;
								}
								add1(_1message);
								if (t % 1000000 == 0)
									std::cout << t << std::endl;
							}
							if (total_break)
							{
								break;
							}
						}
						add1(message);

						if (total_break)
						{
							break;
						}
					}
					if (total_break)
					{
						break;
					}
					//возможно (точно) нужен костыль "если всё прошли, а коллизия не найдена, то i = 0, и берём другие месседжи"
					message_first = _1message_last;
					message_last = (unsigned long long int)(message_first + (unsigned long long int)ceil((pow(2, (n + phase) / 2 - 1)) / (pow(2, phase) - 2 * step)));
					number2array(message_first, message);
					std::cout << "   Change set M from " << message_first << " to " << message_last << ". Cardinality: " << count_bound(message_first, message_last) << std::endl;

					_1message_first = message_last;
					_1message_last = (unsigned long long int)(_1message_first + (unsigned long long int)ceil((pow(2, (n - phase) / 2 + 1)) / (pow(2, phase) - 2 * step)));
					number2array(_1message_first, _1message);
					std::cout << "   Change set M' from " << _1message_first << " to " << _1message_last << ". Cardinality: " << count_bound(_1message_first, _1message_last) << std::endl;
					i = 0;
					std::cout << "recount" << std::endl;
				}
			}
		}
		//TODO: последняя фаза // вроде как сделано
		message_first = 0;
		message_last = message_first + (unsigned long long int)ceil(pow(2, n/2));
		number2array(message_first, message);
		uint8_t* message1 = new uint8_t[LEN];
		memcpy(message1, message, LEN);
		uint8_t* h1_last = A.at(0);
		uint8_t* h2_last = A.at(1);
		bool total_break = false;
		for (size_t i = 0; i < count_bound(message_first, message_last); i++)
		{
			memcpy(h1, h1_last, LEN);
			gN(h1, message, N);
			for (size_t j = 0; j < count_bound(message_first, message_last), i != j; j++)
			{
				memcpy(h2, h2_last, LEN);
				gN(h2, message1, N);
				if (equal_array(h1, h2))
				{
					A.erase(A.begin());
					A.erase(A.begin());
					temp_A.push_back(h1); //добавили хэш для следующего джампа
					std::pair<uint8_t*, uint8_t*> h1_and_x1(h1_last, message);
					std::pair<uint8_t*, uint8_t*> h2_and_x2(h2_last, message1);
					B_in_jump.push_back(h1_and_x1);
					B_in_jump.push_back(h2_and_x2);
					total_break = true;

					std::cout << "   colliding pairs h1_last: ";
					print_array(h1_last);
					std::cout << "   and message: ";
					print_array(message);
					std::cout << "   colliding pairs h2_last: ";
					print_array(h2_last);
					std::cout << "   and message: ";
					print_array(message1);
					std::cout << "   result: ";
					print_array(h1);
					std::cout << std::endl;

					break;
				}
				add1(message1);
			}
			add1(message);
			if (total_break)
			{
				break;
			}

		}
		delete[] message1;
		delete[] h1_last;
		delete[] h2_last;

		add64(N);
		B.push_back(B_in_jump);
		B_in_jump.clear();
		A.clear();
		for (auto const& hash : temp_A)
		{
			A.push_back(hash);
		}
		temp_A.clear();
	}

	//TODO: последний jump, где d = 1 // вроде как сделано
	message_first = 0;
	message_last = message_first + (unsigned long long int)ceil(pow(2, n / 2));
	number2array(message_first, message);
	uint8_t* message1 = new uint8_t[LEN];
	memcpy(message1, message, LEN);
	uint8_t* h1_last = A.at(0);
	uint8_t* h2_last = A.at(1);

	bool total_break = false;
	for (size_t i = 0; i < count_bound(message_first, message_last); i++)
	{
		memcpy(h1, h1_last, LEN);
		gN(h1, message, N);
		for (size_t j = 0; j < count_bound(message_first, message_last); j++)
		{
			if (i == j)
			{
				add1(message1);
				continue;
			}
			memcpy(h2, h2_last, LEN);
			gN(h2, message1, N);
			if (equal_array(h1, h2))
			{
				std::pair<uint8_t*, uint8_t*> h1_and_x1(h1_last, message);
				std::pair<uint8_t*, uint8_t*> h2_and_x2(h2_last, message1);
				std::vector <std::pair<uint8_t*, uint8_t*>> B1;
				B1.push_back(h1_and_x1);
				B1.push_back(h2_and_x2);
				B.push_back(B1);
				std::vector <std::pair<uint8_t*, uint8_t*>> B0;
				uint8_t* zero_message = new uint8_t[8];
				std::fill_n(zero_message, 8, 0);
				std::pair<uint8_t*, uint8_t*> h0(h1, zero_message);
				B0.push_back(h0);
				B.push_back(B0);
				total_break = true;

				std::cout << "   colliding pairs h1_last: ";
				print_array(h1_last);
				std::cout << "   and message: ";
				print_array(message);
				std::cout << "   colliding pairs h2_last: ";
				print_array(h2_last);
				std::cout << "   and message: ";
				print_array(message1);
				std::cout << "   result: ";
				print_array(h1);
				std::cout << std::endl;

				break;
			}
			add1(message1);
		}
		add1(message);
		if (total_break)
		{
			break;
		}

	}
	delete[] message1;
	delete[] h1_last;
	delete[] h2_last;

	add64(N);
	B.push_back(B_in_jump);
	B_in_jump.clear();
	A.clear();
	for (auto const& hash : temp_A)
	{
		A.push_back(hash);
	}
	temp_A.clear();
}

void add_one(uint8_t* b)
{
	uint8_t* a = new uint8_t[8];
	for (int i = 0; i < 8; i++)
	{
		a[i] = 7 - i;
	}
	/*for (int i = 0; i < 8; i++) {
		b[i] = b[i] + 1;
		cout << (int)b[i] << " ";
	}
	cout << endl;*/
	print_array(b);
	memcpy(b, a, 8);
	print_array(b);
	a[1] = 100;
	print_array(b);
	a = NULL;
	print_array(b);
}

void generate_tables()
{
	std::ofstream file;
	file.open("precalctable.txt");

	for (int j = 0; j < 4; j++)
	{
		file << "{"; //fprintf(file, "{");
			for (int i = 0; i < 16; i++)
			{
				if (i)
				{
					file << ", "; //fprintf(file, ", ");
				}
				if (i % 4 == 0)
				{
					file << "\n\t"; //fprintf(file, "\n\t");
				}
				uint8_t t = PI[i];
				uint16_t a = 0x00;
				for (int k = 0; k < 4; k++)
				{
					if (t & 1)
					{
						a ^= A[15 - 4 * j - k];
					}
					t >>= 1;
				}
				file << "0x";//fprintf(file, "0x");
				file << std::hex << a; //fprintf(file, "%04hx", a);
			}
		if (j != 3)
		{
			file << "\n},\n"; //fprintf(file, ",\n}\n");
		}
	}
	file << "\n}\n"; //fprintf(file, "\n}\n");
	
}

int main()
{
	//generate_tables();

	/*uint8_t messagef[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	uint8_t message0[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t message_rand[8] = {0x01, 0xd4, 0x44, 0x90, 0x7e, 0xfb, 0x8c, 0xf7 };
	HashFunctionMora hash_func(8);
	hash_func.print_array(message_rand);
	hash_func.print_array(hash_func.gN(message0, message_rand, message0));*/
	
	//diamond_structure(4);


	/*uint8_t* a = new uint8_t[8];
	uint8_t* b = new uint8_t[8];
	uint8_t* res = new uint8_t[8];
	for (int i = 0; i < 8; i++)
	{
		a[i] = i + 1;
		b[i] = 8 - i;
	}
	X(a, b, res);
	print_array(a);
	print_array(b);
	std::cout << "X operation" << std::endl;
	print_array(res);
	std::cout << std::endl;

	uint8_t s[8] = { 0xf2, 0x52, 0xb7, 0x16, 0x8d, 0x7d, 0x7f, 0x71 };
	S(s);
	std::cout << "S operation" << std::endl;
	print_array(s);
	uint8_t* s1 = new uint8_t[8];
	s1[0] = 0xf2; s1[1] = 0x52; s1[2] = 0xb7; s1[3] = 0x16; s1[4] = 0x8d; s1[5] = 0x7d; s1[6] = 0x7f; s1[7] = 0x71;
	S(s1);
	std::cout << "S operation" << std::endl;
	print_array(s1);
	std::cout << std::endl;

	uint8_t p[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
	P(p);
	std::cout << "P operation" << std::endl;
	print_array(p);
	uint8_t p1[8] = { 0xa1, 0xc1, 0x38, 0x92, 0x6b, 0x8b, 0x8a, 0x89 };
	P(p1);
	std::cout << "P operation" << std::endl;
	print_array(p1);
	std::cout << std::endl;

	uint8_t l[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	L(l);
	std::cout << "L operation" << std::endl;
	print_array(l);
	uint8_t l1[8] = { 0xb8, 0x1f, 0xcb, 0xc5, 0x5e, 0x94, 0xf3, 0xcf };
	L(l1);
	std::cout << "L operation" << std::endl;
	print_array(l1);
	std::cout << std::endl;

	uint8_t h[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t N[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t gn[8] = {0x01, 0xd4, 0x44, 0x90, 0x7e, 0xfb, 0x8c, 0xf7 };
	auto start = high_resolution_clock::now();
	gN(h, gn, N);
	std::cout << "gN operation" << std::endl;
	print_array(h);
	print_array(gn);
	print_array(N);
	std::cout << std::endl;
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	std::cout << std::dec << duration.count() << " microseconds" << std::endl;*/


	/*std::vector<uint8_t*> vec;
	uint8_t* avec = new uint8_t[8];
	for (int i = 0; i < 8; i++)
	{
		avec[i] = i;
	}
	vec.push_back(avec);
	print_array(vec.at(0));
	for (int i = 0; i < 8; i++)
	{
		avec[i] = 66;
	}
	print_array(vec.at(0));*/

	/*uint8_t* a = new uint8_t[8];
	for (int i = 0; i < 8; i++)
	{
		a[i] = i;
	}
	a[7] = 255;
	print_array(a);
	add1(a);
	print_array(a);

	unsigned long long int b = 64;
	number2array(b, a);
	print_array(a);*/
	
	/*vector<uint8_t*> a;
	uint8_t b[4] = {1, 2, 3, 4};
	a.push_back(b);
	b[0] = 5;
	a.push_back(b);
	b[1] = 6;
	a.push_back(b);

	for (auto const& value : a)
		print_array(value);
	cout << endl;
	a.erase(a.begin() + 1);
	for (auto const& value : a)
		print_array(value);
	cout << endl;
	a.erase(a.begin() + 1);
	for (auto const& value : a)
		print_array(value);
	cout << endl;*/

	uint8_t h[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t N[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t gn[8] = { 0x01, 0xd4, 0x44, 0x90, 0x7e, 0xfb, 0x8c, 0xf7 };
	auto start = high_resolution_clock::now();
	fast_gN(h, gn, N);
	std::cout << "gN operation" << std::endl;
	print_array(h);
	print_array(gn);
	print_array(N);
	std::cout << std::endl;
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	std::cout << std::dec << duration.count() << " microseconds" << std::endl;
}