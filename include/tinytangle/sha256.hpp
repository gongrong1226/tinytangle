#pragma once

#include <string>

namespace tangle
{

	class SHA256
	{
	protected:
		typedef unsigned char uint8;
		typedef unsigned int uint32;
		typedef unsigned long long uint64;

		const static uint32 sha256_k[];
		static const unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);
	public:
		//常量初始化，自然数中前8个质数（2,3,5,7,11,13,17,19）的平方根的小数部分取前32bit 
		void init();
		void update(const unsigned char *message, unsigned int len);
		void final(unsigned char *digest);
		static const unsigned int DIGEST_SIZE = (256 / 8);

	protected:
		void transform(const unsigned char *message, unsigned int block_nb);
		unsigned int m_tot_len;
		unsigned int m_len;
		unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
		uint32 m_h[8];
	};

	//使用sha256将输入字符串提取hash并转换成字符串
	std::string sha256(const std::string& input);

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

}// tinychain