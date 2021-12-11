#include <iostream>
#include "HashFunctionMora.h"

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

int main()
{
	uint8_t messagef[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	uint8_t message0[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t message_rand[8] = {0x01, 0xd4, 0x44, 0x90, 0x7e, 0xfb, 0x8c, 0xf7 };
	HashFunctionMora hash_func(8);
	hash_func.print_array(message_rand);
	hash_func.print_array(hash_func.gN(message0, message_rand, message0));
}