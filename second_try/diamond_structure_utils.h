#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#pragma once

int compare_arr(const uint8_t* a, const uint8_t* b, const int len);
bool equal_array(const uint8_t* a, const uint8_t* b, const int len);
std::string arr_to_string(const uint8_t* arr, const int len);
uint8_t* string_to_arr(const std::string str);
void clear_vec(std::vector<uint8_t*>& vec);
std::string arr_to_hexstring(const uint8_t* arr, const size_t len);
