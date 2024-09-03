
#include "../../include/Cryptography/Crypt.h"
#include <stdint.h>
#include <string.h>

namespace Common
{
	namespace Cryptography
	{
		Crypt::Crypt(uint32_t key)
		{
			KeySetup(key);
			isUsed = true;
		}

		void Crypt::RC5KeySetup()
		{
			//unsigned char K[16] = { 0x8c, 0x63, 0xc5, 0xa3, 0x88, 0x9a, 0xdb, 0xa5, 0xd1, 0xb2, 0x49, 0x17, 0xb6, 0x94, 0x2c, 0xa1 }; // Older versions
			unsigned char K[16] = { 0x3d, 0x63, 0xc5, 0xa3, 0x6d, 0x9a, 0xdb, 0xa5, 0xd1, 0xb2, 0x7a, 0x17, 0xb6, 0x56, 0x2c, 0xba }; // Newer versions
			uint32_t A, B, L[4];
			int i, j, k, l = 0;
			char UserKeyBytes[4];
			UserKeyBytes[0] = UserKey & 0xFF;
			UserKeyBytes[1] = UserKey >> 8 & 0xFF;
			UserKeyBytes[2] = UserKey >> 16 & 0xFF;
			UserKeyBytes[3] = UserKey >> 24 & 0xFF;
			for (i = 15, L[3] = 0; i >= 0; i--)
			{
				L[i / 4] = (L[i / 4] << 8) + K[i] + UserKeyBytes[l];
				if (++l > 3) l = 0;
			}
			for (RC5S[0] = 0x5163, i = 1; i < 26; i++)
				RC5S[i] = RC5S[i - 1] + 0x79b9;
			for (A = B = i = j = k = 0; k < 26 * 3; k++, i = (i + 26 / 2) % 26, j = (j + 1) % 4)
			{
				A = RC5S[i] = ROTL32(RC5S[i] + (A + B), 3);
				B = L[j] = ROTL32(L[j] + (A + B), (A + B));
			}
		}

		void Crypt::RC6KeySetup()
		{
			//unsigned char K[32] = {0x31, 0xB7, 0x7C, 0x98, 0x37, 0x5B, 0xD5, 0xE3, 0x71, 0x92, 0x33, 0x6A, 0x7B, 0xE6, 0xCC, 0xEB,
			//	0x39, 0x9A, 0x77, 0xBC, 0x31, 0x5D, 0xE7, 0x39, 0x53, 0x32, 0x54, 0x88, 0x66, 0xD3, 0xCE, 0x61}; // older clients

			unsigned char K[32] = { 0x76, 0xb7, 0x4b, 0x98, 0x4c, 0x5b, 0xd5, 0xe3, 0xc1, 0x92, 0x33, 0x6a, 0x7b, 0xe6, 0xcc, 0xeb, 0x17, 0x9a, 0x77, 0xbc, 0x31, 0x5d, 0xe7, 0x39, 0xa9, 0x32, 0x54, 0x88, 0x66, 0xd3, 0xce, 0x43 };
			uint32_t A, B, L[8];
			int i, j, k, l = 0;
			char UserKeyBytes[4];
			UserKeyBytes[0] = UserKey & 0xFF;
			UserKeyBytes[1] = UserKey >> 8 & 0xFF;
			UserKeyBytes[2] = UserKey >> 16 & 0xFF;
			UserKeyBytes[3] = UserKey >> 24 & 0xFF;
			for (i = 31, L[7] = 0; i >= 0; i--)
			{
				L[i / 4] = (L[i / 4] << 8) + K[i] + UserKeyBytes[l];
				if (++l > 3) l = 0;
			}
			for (RC6S[0] = 0xb7e15163, i = 1; i < 84; i++)
				RC6S[i] = RC6S[i - 1] + 0x9e3779b9;
			for (A = B = i = j = k = 0; k < 84 * 3; k++, i = (i + 1) % 84, j = (j + 1) % 8)
			{
				A = RC6S[i] = ROTL32(RC6S[i] + A + B, 3);
				B = L[j] = ROTL32(L[j] + A + B, A + B);
			}
		}

		void Crypt::KeySetup(uint32_t key)
		{
			UserKey = key;
			RC5KeySetup();
			RC6KeySetup();
		}

		void Crypt::RC5Encrypt32(const void* source, void* destination, int size)
		{
			uint16_t A, B, * src = (uint16_t*)source, * dst = (uint16_t*)destination;
			int i, j;
			if (source != destination && size % 4) memcpy((char*)destination + size - size % 4, (char*)source + size - size % 4, size % 4);
			for (j = size / 4; j > 0; j--, src += 2, dst += 2)
			{
				A = src[0] + RC5S[0];
				B = src[1] + RC5S[1];
				for (i = 1; i <= 12; i++)
				{
					A = ROTL16(A ^ B, B) + RC5S[2 * i];
					B = ROTL16(B ^ A, A) + RC5S[2 * i + 1];
				}
				dst[0] = A ^ UserKey;
				dst[1] = B ^ UserKey;
			}
		}

		void Crypt::RC5Decrypt32(const void* source, void* destination, int size)
		{
			uint16_t A, B, * src = (uint16_t*)source, * dst = (uint16_t*)destination;
			int i, j;
			if (source != destination && size % 4) memcpy((char*)destination + size - size % 4, (char*)source + size - size % 4, size % 4);
			for (j = size / 4; j > 0; j--, src += 2, dst += 2)
			{
				A = src[0] ^ UserKey;
				B = src[1] ^ UserKey;
				for (i = 12; i > 0; i--)
				{
					B = ROTR16(B - RC5S[2 * i + 1], A) ^ A;
					A = ROTR16(A - RC5S[2 * i], B) ^ B;
				}
				dst[0] = A - RC5S[0];
				dst[1] = B - RC5S[1];
			}
		}

		void Crypt::RC5Encrypt64(const void* source, void* destination, int size)
		{
			uint32_t A, B, * src = (uint32_t*)source, * dst = (uint32_t*)destination;
			int i, j;
			for (j = size / 8; j > 0; j--, src += 2, dst += 2)
			{
				A = src[0] + RC5S[0];
				B = src[1] + RC5S[1];
				for (i = 1; i <= 12; i++)
				{
					A = ROTL32(A ^ B, B) + RC5S[2 * i];
					B = ROTL32(B ^ A, A) + RC5S[2 * i + 1];
				}
				dst[0] = A ^ UserKey;
				dst[1] = B ^ UserKey;
			}
			RC5Encrypt32((uint32_t*)source + (size - size % 8) / 4, (uint32_t*)destination + (size - size % 8) / 4, size % 8);
		}

		void Crypt::RC5Decrypt64(const void* source, void* destination, int size)
		{
			uint32_t A, B, * src = (uint32_t*)source, * dst = (uint32_t*)destination;
			int i, j;
			for (j = size / 8; j > 0; j--, src += 2, dst += 2)
			{
				A = src[0] ^ UserKey;
				B = src[1] ^ UserKey;
				for (i = 12; i > 0; i--)
				{
					B = ROTR32(B - RC5S[2 * i + 1], A) ^ A;
					A = ROTR32(A - RC5S[2 * i], B) ^ B;
				}
				dst[0] = A - RC5S[0];
				dst[1] = B - RC5S[1];
			}
			RC5Decrypt32((uint32_t*)source + (size - size % 8) / 4, (uint32_t*)destination + (size - size % 8) / 4, size % 8);
		}

		void Crypt::RC6Encrypt128(const void* source, void* destination, int size)
		{
			uint32_t A, B, C, D, t, u, x, * src = (uint32_t*)source, * dst = (uint32_t*)destination;
			int i, j;
			for (j = size / 16; j > 0; j--, src += 4, dst += 4)
			{
				A = src[0];
				B = src[1] + RC6S[0];
				C = src[2];
				D = src[3] + RC6S[1];
				for (i = 2; i <= 2 * 40; i += 2)
				{
					t = ROTL32(B * (2 * B + 1), 5);
					u = ROTL32(D * (2 * D + 1), 5);
					A = ROTL32(A ^ t, u) + RC6S[i];
					C = ROTL32(C ^ u, t) + RC6S[i + 1];
					x = A;
					A = B;
					B = C;
					C = D;
					D = x;
				}
				dst[0] = (A + RC6S[2 * 40 + 2]) ^ UserKey;
				dst[1] = B ^ UserKey;
				dst[2] = (C + RC6S[2 * 40 + 3]) ^ UserKey;
				dst[3] = D ^ UserKey;
			}
			RC5Encrypt64((uint32_t*)source + (size - size % 16) / 4, (uint32_t*)destination + (size - size % 16) / 4, size % 16);
		}

		void Crypt::RC6Decrypt128(const void* source, void* destination, int size)
		{
			uint32_t A, B, C, D, t, u, x, * src = (uint32_t*)source, * dst = (uint32_t*)destination;
			int i, j;
			for (j = size / 16; j > 0; j--, src += 4, dst += 4)
			{
				A = (src[0] ^ UserKey) - RC6S[2 * 40 + 2];
				B = src[1] ^ UserKey;
				C = (src[2] ^ UserKey) - RC6S[2 * 40 + 3];
				D = src[3] ^ UserKey;
				for (i = 2 * 40; i >= 2; i -= 2)
				{
					x = D;
					D = C;
					C = B;
					B = A;
					A = x;
					u = ROTL32(D * (2 * D + 1), 5);
					t = ROTL32(B * (2 * B + 1), 5);
					C = ROTR32(C - RC6S[i + 1], t) ^ u;
					A = ROTR32(A - RC6S[i], u) ^ t;
				}
				dst[0] = A;
				dst[1] = B - RC6S[0];
				dst[2] = C;
				dst[3] = D - RC6S[1];
			}
			RC5Decrypt64((uint32_t*)source + (size - size % 16) / 4, (uint32_t*)destination + (size - size % 16) / 4, size % 16);
		}
	}
}
