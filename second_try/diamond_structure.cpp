#include "diamond_structure.h"

std::stringstream stringstream_with_logs;

int main(int argc, char** argv)
{
	int n = -1;
	int d = -1;

	if (!process_args(&d, &n, argc, argv))
	{
		return -1;
	}

	HashFunctionMora hf(8, n);

	log("Start diamond structure construction with d = " + std::to_string(d) + ", and n = " + std::to_string(n), true);
	diamond_structure(d, hf);
}

bool process_args(int* d, int* n, const int argc, char** argv)
{
	const std::string info_message = "Incorrect input. Need somethig like this:\nsecond_try.exe -d <integer> [-n <integer>]";

	if ((argc != 3) && (argc != 5)) {
		log(info_message, false);
		return false;
	}
	if (argv[1][0] != '-' || argv[1][1] != 'd' || argv[1][2] != '\0')
	{
		log(info_message, false);
		return false;
	}
	*d = atoi(argv[2]);
	if (argc == 5)
	{
		if (argv[3][0] != '-' || argv[3][1] != 'n' || argv[3][2] != '\0')
		{
			log(info_message, false);
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
		log(info_message, false);
		return false;
	}
	return true;
}

void diamond_structure(const int d, HashFunctionMora& hf)
{
	const int n = hf.get_HASH_LEN();
	const int MESSAGE_LEN = 8;

	if (d <= 1 || d >= n * 8 / 2)
	{
		log("Incorrect input d = " + std::to_string(d) + ". d must be between 2 and " + std::to_string(n * 8 / 2), false);
		return;
	}

	std::vector<uint8_t*> A;
	std::vector<uint8_t*> tmp_A;
	std::vector<std::vector<std::pair<std::string, std::string>>> B;
	std::vector<uint8_t*> M;
	HContainer hcontainer(n, MESSAGE_LEN);
	std::vector<uint8_t*> M1;
	std::vector<std::pair<std::string, std::string>> B_in_jump;

	const unsigned long init_hash_count = (unsigned long)pow(2, d);
	srand((unsigned int)time(nullptr));
	generate_blocks(A, init_hash_count, n, true);

	uint8_t* N = new uint8_t[MESSAGE_LEN];
	std::fill_n(N, MESSAGE_LEN, 0);

	for (size_t jump = d; jump >= 2; jump--)
	{
		log("jump: " + std::to_string(jump), true);

		unsigned long m_cardinality = (unsigned long)pow(2, ((8 * static_cast<unsigned long long>(n) - jump) / 2) - 1);
		log("Generating set M ...", false);
		auto start = std::chrono::high_resolution_clock::now();
		generate_blocks(M, m_cardinality, MESSAGE_LEN, false);
		log_and_count_time_of_exec(start, "M", M.size());

		log("Generating set H ...", false);
		start = std::chrono::high_resolution_clock::now();
		hcontainer.calculate_H(A, M, hf, N);
		log_and_count_time_of_exec(start, "H", hcontainer.size());

		for (size_t phase = jump; phase >= 2; phase--)
		{
			log(" phase: " + std::to_string(phase), true);

			const size_t max_step = (size_t)pow(2, phase - 2);
			for (size_t step = 0; step < max_step; step++)
			{
				bool collision_found = false;
				while (!collision_found)
				{
					log("  step: " + std::to_string(step), true);

					const unsigned long long m1_cardinality =
						(unsigned long long)std::ceil(pow(2, (8 * static_cast<unsigned long long>(n) - phase) / 2 + 1) / (pow(2, phase) - 2 * step));
					log("Generating set M1 ...", false);
					start = std::chrono::high_resolution_clock::now();
					generate_blocks(M1, m1_cardinality, MESSAGE_LEN, false);
					log_and_count_time_of_exec(start, "M1", M1.size());

					log("Searching for collision...", false);
					start = std::chrono::high_resolution_clock::now();
					const auto* collision = hcontainer.get_collision(A, M1, hf, N);
					if (collision != nullptr)
					{
						log_and_count_time_of_collision_searching(start);
						collision_found = true;

						const auto p1 = std::get<0>(*collision);
						const auto p2 = std::get<1>(*collision);
						const uint8_t* h1 = p1.first;
						const uint8_t* h2 = p2.first;
						const uint8_t* m1 = p1.second;
						const uint8_t* m2 = p2.second;
						const std::string hash_res = std::get<2>(*collision);

						tmp_A.push_back(string_to_arr(hash_res));
						B_in_jump.push_back(std::make_pair(arr_to_string(h1, n), arr_to_string(m1, MESSAGE_LEN)));
						B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(m2, MESSAGE_LEN)));

						log("h1: " + arr_to_hexstring(h1, n) + ", m1: " + arr_to_hexstring(m1, MESSAGE_LEN), true);
						log("h2: " + arr_to_hexstring(h2, n) + ", m2: " + arr_to_hexstring(m2, MESSAGE_LEN), true);

						A.erase(std::remove(A.begin(), A.end(), h1), A.end());
						delete[] h1;
						A.erase(std::remove(A.begin(), A.end(), h2), A.end());
						delete[] h2;
						log("Cardinality of A: " + std::to_string(A.size()), true);

						M.insert(M.end(), M1.begin(), M1.end());
						log("Cardinality of M after union with M1: " + std::to_string(M.size()), true);
						M1.clear();
						hcontainer.clear();

						if (phase != 2)
						{
							log("Recalc of set H ...", false);
							start = std::chrono::high_resolution_clock::now();
							hcontainer.calculate_H(A, M, hf, N);
							log_and_count_time_of_exec(start, "H", hcontainer.size());
						}
					}
					if (!collision_found)
					{
						clear_vec(M1);
					}
				}
			}
		}

		log(" phase: 1", true);
		process_last_two_hashes_in_jump(A, M, hcontainer, B_in_jump, tmp_A, hf, N, MESSAGE_LEN);

		clear_vec(M);
		hcontainer.clear();

		B.push_back(B_in_jump);
		B_in_jump.clear();
		for (auto& h : tmp_A)
		{
			A.push_back(h);
		}
		tmp_A.clear();
		add64(N);
	}

	log("jump: 1", true);
	process_last_two_hashes_in_jump(A, M, hcontainer, B_in_jump, tmp_A, hf, N, MESSAGE_LEN);
	B.push_back(B_in_jump);
	create_result_and_log_files(B);

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

void process_last_two_hashes_in_jump(std::vector<uint8_t*>& A,
	std::vector<uint8_t*>& M,
	HContainer& hcontainer,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN)
{
	const int n = hf.get_HASH_LEN();

	while (A.size())
	{
		log("Generating set M ...", false);
		auto start = std::chrono::high_resolution_clock::now();
		generate_blocks(M, M.size(), (size_t)pow(2, (8 * n) / 2), MESSAGE_LEN, false);
		log_and_count_time_of_exec(start, "M", M.size());
		process_last_two_hashes(A, M, hcontainer, B_in_jump, tmp_A, hf, N, MESSAGE_LEN);

		if (A.size())
		{
			clear_vec(M);
			hcontainer.clear();
		}
	}
}

void process_last_two_hashes(std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M,
	HContainer& hcontainer,
	std::vector<std::pair<std::string, std::string>>& B_in_jump,
	std::vector<uint8_t*>& tmp_A,
	HashFunctionMora& hf,
	const uint8_t* N,
	const int MESSAGE_LEN)
{
	const int n = hf.get_HASH_LEN();
	std::vector<uint8_t*> A1;
	A1.push_back(A.at(0));

	log("Generating set H ...", false);
	auto start = std::chrono::high_resolution_clock::now();
	hcontainer.calculate_H(A1, M, hf, N);
	log_and_count_time_of_exec(start, "H", hcontainer.size());

	std::vector<uint8_t*> A2;
	A2.push_back(A.at(1));
	log("Searching for collision...", false);
	start = std::chrono::high_resolution_clock::now();
	const auto collision = hcontainer.get_collision(A2, M, hf, N);
	if (collision != nullptr)
	{
		log_and_count_time_of_collision_searching(start);
		const auto p1 = std::get<0>(*collision);
		const auto p2 = std::get<1>(*collision);
		const std::string hash_res = std::get<2>(*collision);
		const uint8_t* h1 = p1.first;
		const uint8_t* h2 = p2.first;
		const uint8_t* m1 = p1.second;
		const uint8_t* m2 = p2.second;

		tmp_A.push_back(string_to_arr(hash_res));
		B_in_jump.push_back(std::make_pair(arr_to_string(h1, n), arr_to_string(m1, MESSAGE_LEN)));
		B_in_jump.push_back(std::make_pair(arr_to_string(h2, n), arr_to_string(m2, MESSAGE_LEN)));

		log("h1: " + arr_to_hexstring(h1, n) + ", m1: " + arr_to_hexstring(m1, 8), true);
		log("h2: " + arr_to_hexstring(h2, n) + ", m2: " + arr_to_hexstring(m2, 8), true);

		delete[] h1;
		delete[] h2;
		A1.clear();
		A2.clear();
		A.clear();
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

void create_result_and_log_files(const std::vector<std::vector<std::pair<std::string, std::string>>>& B)
{
	time_t t = std::time(nullptr);
	struct tm* time_info = localtime(&t);
	std::stringstream collision_file_name;
	collision_file_name << std::put_time(time_info, "%d-%m-%Y-%H-%M-%S_collision.txt");

	std::ofstream collisions_file;
	collisions_file.open(collision_file_name.str());
	size_t jump = B.size();
	for (const auto& vec : B)
	{
		collisions_file << "jump �" << jump << ":" << "\n";
		int num = 1;
		for (const auto& pair : vec)
		{
			auto h = pair.first;
			auto m = pair.second;
			collisions_file << "(h" << num << ", x" << num << "): (" << arr_to_hexstring(string_to_arr(h), h.length()) << ", "
				<< arr_to_hexstring(string_to_arr(m), m.length()) << ")" << "\n";
			num++;
		}
		collisions_file << "\n";
		jump--;
	}
	collisions_file.close();

	std::stringstream log_file_name;
	log_file_name << std::put_time(time_info, "%d-%m-%Y-%H-%M-%S_log.txt");
	std::ofstream log_file;
	log_file.open(log_file_name.str());

	log_file << stringstream_with_logs.str();
	log_file.close();
}

void log(const std::string message, const bool to_log_file)
{
	std::cout << message << "\n";
	if (to_log_file)
	{
		stringstream_with_logs << message << "\n";
	}
}

void log_and_count_time_of_exec(const std::chrono::high_resolution_clock::time_point start, const std::string set_name, const size_t set_size)
{
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	log("Set " + set_name + " of cardinality " + std::to_string(set_size) +
		" generated in " + std::to_string(duration.count()) + " microseconds", true);
}

void log_and_count_time_of_collision_searching(const std::chrono::high_resolution_clock::time_point start)
{
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	log("Collision found in " + std::to_string(duration.count()) + " microseconds", true);
}
