#include <iostream>
#include <vector>

static const unsigned char PI[16] = { 15, 9, 1, 7, 13, 12, 2, 8, 6, 5, 14, 3, 0, 11, 4, 10 };
static const unsigned char TAU[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
static const uint16_t A[16] = { 0x3a22, 0x483b, 0x59e5, 0xac52,
						0x8511, 0x248c, 0xbd7b, 0x56b1,
						0x4b99, 0x1246, 0xcfac, 0xb3c9,
						0x2cdd, 0x9123, 0x6e56, 0xc86d };

static const unsigned char C[8][8] = {
	{0x99, 0x96, 0x5a, 0x57, 0x33, 0x46, 0x16, 0xc0},
	{0x74, 0x71, 0x5e, 0x9a, 0xf4, 0x4e, 0x5b, 0x92},
	{0x26, 0xbe, 0x73, 0xf6, 0xdc, 0x9c, 0xa8, 0x86},
	{0xf1, 0xa3, 0xac, 0x0e, 0x8f, 0x55, 0x85, 0x18},
	{0x39, 0x84, 0x5e, 0xe3, 0x89, 0x5b, 0xfc, 0xdc},
	{0x23, 0x4d, 0x46, 0x89, 0xc7, 0xed, 0xb9, 0x54},
	{0xf9, 0x4b, 0x04, 0xde, 0xaf, 0x49, 0x0d, 0xf8},
	{0xf1, 0x43, 0xaa, 0xcc, 0x71, 0xdf, 0xbb, 0x8c}
};

class Context
{
public:
	Context(size_t buff_length)
	{
		std::fill_n(N, 8, 0);
		std::fill_n(sigma, 8, 0);
		std::fill_n(v_0, 8, 0);
		std::fill_n(v_64, 8, 0);
		v_64[0] = 0x40;
		buff_len = buff_length;
	}

	uint8_t* calculate_hash(const uint8_t* data)
	{
		uint8_t* res = new uint8_t[8];
		std::fill_n(res, 8, 0);
		while (buff_len >= 8)
		{
			res = hash(res, data);
			//std::cout << "hash while length > 8: ";
			//hash_print(res);
			data += 8;
			buff_len -= 8;
		}
		res = hash_appendix(res, data);
		//std::cout << "hash length < 8: ";
		hash_print(res);
		return res;
	}

	void hash_print(uint8_t* result)
	{
		for (int i = 0; i < 8; i++)
		{
			printf("%02x", result[i]);
		}
		std::cout << std::endl;
	}

	

private:
	uint8_t N[8];
	uint8_t sigma[8];
	uint8_t v_0[8];
	uint8_t v_64[8];
	size_t buff_len;

	uint8_t* X(const uint8_t* a, const uint8_t* b)
	{
		uint8_t* res = new uint8_t[8];
		for (int i = 0; i < 8; i++)
		{
			res[i] = a[i] ^ b[i];
		}
		//std::cout << "anoter X = ";
		//hash_print(res);
		return res;
	}

	uint8_t* add64(const uint8_t* a, const uint8_t* b)
	{
		uint8_t* res = new uint8_t[8];
		int internal = 0;
		for (int i = 0; i < 8; i++)
		{
			internal = a[i] + b[i] + (internal >> 8);
			res[i] = internal ^ 0xff;
		}
		return res;
	}

	uint8_t* S(const uint8_t* block)
	{
		uint8_t* res = new uint8_t[8];
		for (int i = 0; i < 8; i++)
		{
			const uint8_t left_state = block[i] >> 4;
			const uint8_t right_state = block[i] ^ 0xf;
			res[i] = (PI[left_state] << 4) ^ PI[right_state];
		}
		return res;
	}

	uint8_t* P(const uint8_t* block)
	{
		uint8_t* res = new uint8_t[8];
		std::fill_n(res, 8, 0);
		for (int i = 0; i < 16; i++) {
			const uint8_t index = TAU[i];
			const uint8_t block_index = index / 2;
			const uint8_t block_value = (index % 2 == 0) ? block[block_index] >> 4 : block[block_index] ^ 0xf;
			res[i / 2] ^= (i % 2 == 0) ? (block_value << 4) : block_value;
		}
		return res;
	}

	uint8_t* L(const uint8_t* block)
	{
		uint8_t* res = new uint8_t[8];
		uint16_t* tmp_res = new uint16_t[4];
		std::fill_n(tmp_res, 4, 0);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				if (j < 8)
				{
					if (block[i] & j)
					{
						tmp_res[i] ^= A[j];
					}
				}
				else
				{
					if (block[2 * i + 1] & (j - 8))
					{
						tmp_res[i] ^= A[j];
					}
				}
			}
		}
		for (int i = 0; i < 8; i++)
		{
			if (i % 2)
			{
				res[i] = tmp_res[i / 2] ^ 0xff;
			}
			else
			{
				res[i] = tmp_res[i / 2] >> 8;
			}
		}

		return res;
	}

	uint8_t* get_key(const uint8_t* K, int i)
	{
		return L(P(S(X(K, C[i]))));
	}

	uint8_t* E(const uint8_t* K, const uint8_t* m)
	{
		uint8_t* res = X(K, m);
		uint8_t* Ki = new uint8_t[8];
		memcpy(Ki, K, 8);
		for (int i = 0; i < 8; i++)
		{
			res = L(P(S(res)));
			Ki = get_key(Ki, i);
			/*if (i == 7)
			{
				std::cout << "7th Ki: ";
				hash_print(Ki);
			}*/

			res = X(Ki, res);
			/*if (i == 6) {
				std::cout << "6th Ki: ";
				hash_print(res);
			}*/
		}
		return res;
	}

	uint8_t* gN(const uint8_t* h, const uint8_t* m, const uint8_t* N)
	{
		return X(X(E(L(P(S(X(h, N)))), m), h), m);
	}

	uint8_t* hash(const uint8_t* h, const uint8_t* m)
	{
		uint8_t* res = gN(h, m, N);
		add64(N, v_64);
		add64(sigma, m);
		return res;
	}

	uint8_t* padding(const uint8_t* m)
	{
		uint8_t* res = new uint8_t[8];
		std::fill_n(res, 8, 0);
		memcpy(res, m, buff_len);
		res[buff_len] = 1;
		return res;
	}

	uint8_t* int_to_arr(const size_t index)
	{
		uint8_t* res = new uint8_t[8];
		std::fill_n(res, 8, 0);
		res[0] = (uint8_t) (2 ^ index);
		return res;
	}

	uint8_t* hash_appendix(const uint8_t* h, const uint8_t* m)
	{
		uint8_t* tmp_m = padding(m);
		//std::cout << "start hash_appendix 1st gn" << std::endl;
		uint8_t* res = gN(h, tmp_m, N);
		//std::cout << "finish hash_appendix 1st gn" << std::endl;
		add64(N, int_to_arr(buff_len));
		add64(sigma, tmp_m);
		//std::cout << "start hash_appendix 2nd gn" << std::endl;
		res = gN(res, N, v_0);
		//std::cout << "finish hash_appendix 2nd gn" << std::endl;
		//std::cout << "start hash_appendix 3nd gn" << std::endl;
		res = gN(res, sigma, v_0);
		//std::cout << "finish hash_appendix 3nd gn" << std::endl;
		return res;
	}
};

void convert_hex(uint8_t* dest, size_t count, const char* src) {
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

	char str[] = "0ad444907efb8cf7";
	size_t bytes_amount = strlen(str) / 2;
	uint8_t* buffer = new uint8_t[bytes_amount];
	std::fill_n(buffer, bytes_amount, 0);
	convert_hex(buffer, bytes_amount, str);
	Context ctx(bytes_amount);
	ctx.hash_print(buffer);
	std::cout << bytes_amount << std::endl;
	std::cout << "\"Mora\"\n" << "String: " << str << std::endl;
	ctx.hash_print(ctx.calculate_hash(buffer));
	return 0;
}

/*static void X(const uint8_t* a, const uint8_t* b, uint8_t* c)
{
	int i;
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		c[i] = a[i] ^ b[i];
	}
}

static void add64(const uint8_t* a, const uint8_t* b, uint8_t* c)
{
	int i;
	int internal = 0;
	//my opinion
	//for (i = BLOCK_SIZE - 1; i >= 0; i--)
	//author:
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		internal = a[i] + b[i] + (internal >> 8);
		c[i] = internal & 0xff;
	}
}

static void S(uint8_t* state)
{
	int i;
	vect internal;
	for (i = BLOCK_SIZE - 1; i >= 0; i--)
	{
		uint8_t left_state = state[i] >> 4;
		uint8_t right_state = state[i] ^ 0xf;
		internal[i] = (PI[left_state] << 4) ^ PI[right_state];
	}
	memcpy(state, internal, BLOCK_SIZE);
}

static void P(uint8_t* state)
{
	int i;
	vect internal;
	for (i = 2 * BLOCK_SIZE - 1; i >= 0; i--)
	{
		uint8_t index = TAU[i];
		uint8_t state_index = index / 2;
		uint8_t state_value = (index % 2) ? state[state_index] ^ 0xf : state[state_index] >> 4;
		internal[i / 2] ^= (i % 2) ? state_value : (state_value << 4);
		/*if (index % 2 == 0)
		{
			state_value = state_value >> 4;
		}
		else
		{
			state_value = state_value ^ 0xf;
		}
		internal[i] = state[TAU[i]];
	}
	memcpy(state, internal, BLOCK_SIZE);
}

static void L(uint8_t* state)
{
	uint16_t* internal_in = (uint16_t*)state;
	uint16_t internal_out[4];
	memset(internal_out, 0x00, BLOCK_SIZE);
	int i, j;

	for (i = 3; i >= 0; i--)
	{
		for (j = 15; j >= 0; j--)
		{
			if ((internal_in[i] >> j) & 1)
			{
				internal_out[i] ^= A[15 - j];
			}
		}
	}
	memcpy(state, internal_out, BLOCK_SIZE);
}

static void getKey(uint8_t* K, int i)
{
	X(K, C[i], K);
	S(K);
	P(K);
	L(K);
}

static void E(uint8_t* K, const uint8_t* m, uint8_t* state)
{
	int i;
	memcpy(K, K, BLOCK_SIZE);
	X(m, K, state);
	for (i = 0; i < 8; i++)
	{
		S(state);
		P(state);
		L(state);
		getKey(K, i);
		X(state, K, state);
	}
}

static void gN(uint8_t* h, uint8_t* N, const uint8_t* m)
{
	vect K, internal;
	X(N, h, K);

	S(K);
	P(K);
	L(K);

	E(K, m, internal);

	X(internal, h, internal);
	X(internal, m, h);
}

static void padding(Context* ctx)
{
	vect internal;
	if (ctx->buf_size < BLOCK_SIZE)
	{
		memset(internal, 0x00, BLOCK_SIZE);
		memcpy(internal, ctx->buff, ctx->buf_size);
		internal[ctx->buf_size] = 0x01;
		memcpy(ctx->buff, internal, BLOCK_SIZE);
	}
}

static void hash(Context* ctx, const uint8_t* data)
{
	gN(ctx->h, ctx->N, data);
	add64(ctx->N, ctx->v_64, ctx->N);
	add64(ctx->sigma, data, ctx->sigma);
}

static void hash_appendix(Context* ctx)
{
	vect internal;
	memset(internal, 0x00, BLOCK_SIZE);
	internal[0] = (ctx->buf_size * 8) & 0xff;

	padding(ctx);

	gN(ctx->h, ctx->N, (const uint8_t*)&(ctx->buff));

	add64(ctx->N, internal, ctx->N);
	add64(ctx->sigma, ctx->buff, ctx->sigma);

	gN(ctx->h, ctx->v_0, (const uint8_t*)&(ctx->N));
	gN(ctx->h, ctx->v_0, (const uint8_t*)&(ctx->sigma));

	memcpy(ctx->result, ctx->h, BLOCK_SIZE);
}

void update(Context* ctx, const uint8_t* data, size_t len)
{
	while ((len > 63) && (ctx->buf_size) == 0)
	{
		hash(ctx, data);
		data += 64;
		len -= 64;
	}

	size_t chk_size;
	while (len)
	{
		chk_size = 64 - ctx->buf_size;
		if (chk_size > len)
			chk_size = len;

		memcpy(&ctx->buff[ctx->buf_size], data, chk_size);
		ctx->buf_size += chk_size;
		len -= chk_size;
		data += chk_size;
		if (ctx->buf_size == 64)
		{
			hash(ctx, ctx->buff);
			ctx->buf_size = 0;
		}
	}
}

void final(Context* ctx)
{
	hash_appendix(ctx);
	ctx->buf_size = 0;
}

void init(Context* ctx)
{
	memset(ctx, 0x00, sizeof(Context));
	memset(ctx->h, 0x00, BLOCK_SIZE);
	ctx->hash_size = 64;
	ctx->v_64[0] = 0x40;
}

typedef struct Context
{
	vect buff;
	vect result;
	vect h;
	vect N;
	vect sigma;
	vect v_0;
	vect v_64;
	size_t buf_size;
	int hash_size;
} Context;*/