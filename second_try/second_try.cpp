// second_try.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>

using namespace std;

__int64 X(__int64 k, __int64 a);
__int64 S(__int64 a);
__int64 P(__int64 a);
__int64 L(__int64 a);
__int16 mulOnMatrix(__int16 vector);

__int64 pi[] = { 15, 9, 1, 7, 13, 12, 2, 8, 6, 5, 14, 3, 0, 11, 4, 10 };
__int8 tau[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
__int16 matrixA[] = { (__int16)0x3a22, (__int16)0x483b, (__int16)0x59e5, (__int16)0xac52,
						(__int16)0x8511, (__int16)0x248c, (__int16)0xbd7b, (__int16)0x56b1,
						(__int16)0x4b99, (__int16)0x1246, (__int16)0xcfac, (__int16)0xb3c9,
						(__int16)0x2cdd, (__int16)0x9123, (__int16)0x6e56, (__int16)0xc86d };

__int64 C[] = { (__int64)0xc0164633575a9699, (__int64)0x925b4ef49a5e7174, (__int64)0x86a89cdcf673be26,
					(__int64)0x1885558f0eaca3f1, (__int64)0xdcfc5b89e35e8439, (__int64)0x54b9edc789464d23,
					(__int64)0xf80d49afde044bf9, (__int64)0x8cbbdf71ccaa43f1, (__int64)0xcb43af722cb520b9 };

__int64 X(__int64 k, __int64 a)
{
	return k ^ a;
}

__int64 S(__int64 a)
{
	unsigned __int64 res = 0;
	unsigned __int64 virezatel = 0xf;
	for (int i = 0; i < 16; i++) {
		__int8 part = (__int8)((a & virezatel) >> (i * 4));
		res ^= (pi[part] << (i * 4));
		virezatel <<= 4;
	}
	return res;
}

__int64 P(__int64 a)
{
	__int64 res = 0;
	__int64 virezatel = 0xf;
	for (int i = 0; i < 16; i++) {
		__int8 j = tau[i];
		res ^= ((a & (virezatel << (j * 4))) >> (j * 4)) << (i * 4);
	}
	return res;
}

__int64 L(__int64 a)
{
	__int64 res = 0;
	__int64 virezatel = 0xffff;
	for (int i = 0; i < 4; i++) {
		__int16 mnozitel = (a & (virezatel << (i * 16))) >> (i * 16);
		mnozitel = mulOnMatrix(mnozitel);
		res ^= mnozitel << (i * 16);
	}
	return res;
}

__int16 mulOnMatrix(__int16 vector)
{
	__int16 res = 0;
	__int16 check = 0x8000;
	for (int i = 0; i < 16; i++) {
		if ((vector & check) != 0)
			res ^= matrixA[i];
		check >> 1;
	}
	return res;
}

int main()
{
	__int64 a = 0xf252b7168d7d7f71;
	__int64 sa = S(a);
	cout << std::hex << "S(a): " << sa << endl;
	__int64 psa = P(sa);
	cout << hex << "PS(a): " << psa << endl;
	__int64 lpsa = L(psa);
	cout << hex << "LPS(a): " << lpsa << endl;
}

