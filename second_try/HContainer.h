#include <iostream>
#include <omp.h>
#include <tuple>
#include <vector>
#include "diamond_structure_utils.h"
#include "HashFunctionMora.h"

#pragma once

class HContainer
{
public:
	HContainer(const int hash_len, const int message_len);
	~HContainer();
	void calculate_H(const std::vector<uint8_t*>& A, const std::vector<uint8_t*>& M, HashFunctionMora& hf, const uint8_t* N);
	std::tuple<std::pair<uint8_t*, uint8_t*>, std::pair<uint8_t*, uint8_t*>, std::string>* get_collision(const std::vector<uint8_t*>& A,
		const std::vector<uint8_t*>& M1,
		HashFunctionMora& hf,
		const uint8_t* N);
	void clear();
	size_t size();

private:
	int n;
	int m_len;
	std::vector<uint8_t*>* H;
	std::vector<uint8_t*>* M_bound_with_H;
	std::vector<uint8_t*>* A_bound_with_H;

	void quick_sort(const size_t start, const size_t end, const size_t max_size);
	size_t partition(const size_t start, const size_t end);
	void swap(const size_t i, const size_t j);
	size_t binary_search_for_collision(const size_t left, const size_t right, const uint8_t* hash, const size_t max_size);
	bool suitable_collision(const size_t index, const uint8_t* h, const uint8_t* m);
};
