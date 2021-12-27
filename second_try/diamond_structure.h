#include <chrono>
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
void create_result_and_log_files(const std::vector<std::vector<std::pair<std::string, std::string>>>& B);
std::string arr_to_hexstring(const uint8_t* arr, const size_t len);
void log(const std::string message, const bool to_log_file);
void log_and_count_time_of_exec(const std::chrono::high_resolution_clock::time_point start, const std::string set_name, const size_t set_size);
