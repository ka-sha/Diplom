#include "diamond_structure_utils.h"

int compare_arr(const uint8_t* a, const uint8_t* b, const int len)
{
	int diff = 0;
	for (int i = 0; i < len; i++)
	{
		diff = a[i] - b[i];
		if (diff)
		{
			return diff;
		}
	}
	return 0;
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
