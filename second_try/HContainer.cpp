#include "HContainer.h"

HContainer::HContainer(const int hash_len, const int message_len)
{
	this->n = hash_len;
	this->m_len = message_len;
	this->A_bound_with_H = nullptr;
	this->M_bound_with_H = nullptr;
	this->H = nullptr;
}

HContainer::~HContainer()
{
	delete this->A_bound_with_H;
	delete this->M_bound_with_H;
	delete this->H;
}

void HContainer::calculate_H(const std::vector<uint8_t*>& A, const std::vector<uint8_t*>& M, HashFunctionMora& hf, const uint8_t* N)
{
	const unsigned long long a_size = A.size();
	const unsigned long long m_size = M.size();
	const unsigned long long size = a_size * m_size;

	this->H = new std::vector<uint8_t*>(size);
	this->M_bound_with_H = new std::vector<uint8_t*>(size);
	this->A_bound_with_H = new std::vector<uint8_t*>(size);

	int index = 0;
	for (const auto& h : A)
	{
		for (const auto& m : M)
		{
			uint8_t* hash = new uint8_t[this->n];
			memcpy(hash, h, this->n);
			hf.gN(hash, m, N);
			this->H->at(index) = hash;
			this->A_bound_with_H->at(index) = h;
			this->M_bound_with_H->at(index) = m;
			index++;
		}
	}

	this->quick_sort(0, this->H->size() - 1, this->H->size());
}

void HContainer::quick_sort(const size_t start, const size_t end, const size_t max_size)
{
	size_t index;
	
	if (start < end && end < max_size)
	{
		index = this->partition(start, end);

		#pragma omp parallel sections
		{
			#pragma omp section
			{
				this->quick_sort(start, index - 1, max_size);
			}
			#pragma omp section
			{
				this->quick_sort(index + 1, end, max_size);
			}
		}
	}
}

size_t HContainer::partition(const size_t start, const size_t end)
{
	uint8_t* pivot = this->H->at(end);
	size_t i = (start - 1);

	for (size_t j = start; j <= end - 1; j++)
	{
		if (compare_arr(this->H->at(j), pivot, this->n) < 0)
		{
			i++;
			this->swap(i, j);
		}
	}
	this->swap(i + 1, end);

	return i + 1;
}

void HContainer::swap(const size_t i, const size_t j)
{
	uint8_t* tmp = this->H->at(i);
	this->H->at(i) = this->H->at(j);
	this->H->at(j) = tmp;

	tmp = this->A_bound_with_H->at(i);
	this->A_bound_with_H->at(i) = this->A_bound_with_H->at(j);
	this->A_bound_with_H->at(j) = tmp;

	tmp = this->M_bound_with_H->at(i);
	this->M_bound_with_H->at(i) = this->M_bound_with_H->at(j);
	this->M_bound_with_H->at(j) = tmp;
}

std::tuple<std::pair<uint8_t*, uint8_t*>, std::pair<uint8_t*, uint8_t*>, std::string>* HContainer::get_collision(const std::vector<uint8_t*>& A,
	const std::vector<uint8_t*>& M1,
	HashFunctionMora& hf,
	const uint8_t* N)
{
	uint8_t* hash = new uint8_t[this->n];

	for (const auto& h : A)
	{
		for (const auto& m : M1)
		{
			memcpy(hash, h, this->n);
			hf.gN(hash, m, N);
			size_t collision_index = binary_search_for_collision(0, this->H->size() - 1, hash, this->H->size());
			if (this->suitable_collision(collision_index, h, m))
			{
				delete[] hash;
				return new std::tuple<std::pair<uint8_t*, uint8_t*>, std::pair<uint8_t*, uint8_t*>, std::string>(
					std::make_pair(this->A_bound_with_H->at(collision_index), this->M_bound_with_H->at(collision_index)),
					std::make_pair(h, m),
					arr_to_string(this->H->at(collision_index), this->n));
			}
		}
	}

	delete[] hash;
	return nullptr;
}

size_t HContainer::binary_search_for_collision(const size_t left, const size_t right, const uint8_t* hash, const size_t max_size)
{
	if (right >= left && right < max_size)
	{
		size_t middle = left + (right - left) / 2;
		int diff = compare_arr(this->H->at(middle), hash, this->n);

		if (!diff)
		{
			return middle;
		}

		if (diff > 0)
		{
			return binary_search_for_collision(left, middle - 1, hash, max_size);
		}

		return binary_search_for_collision(middle + 1, right, hash, max_size);
	}

	return -1;
}

bool HContainer::suitable_collision(const size_t index, const uint8_t* h, const uint8_t* m)
{
	return index != -1 && !equal_array(h, this->A_bound_with_H->at(index), this->n) && !equal_array(m, this->M_bound_with_H->at(index), this->n);
}

void HContainer::clear()
{
	this->A_bound_with_H->clear();
	this->M_bound_with_H->clear();
	clear_vec(*this->H);

	delete this->A_bound_with_H;
	delete this->M_bound_with_H;
	delete this->H;
}

size_t HContainer::size()
{
	return this->H->size();
}
