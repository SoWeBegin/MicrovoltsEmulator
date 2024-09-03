// Add original Author (Qweha on GitHub)

#ifndef CRYPTOGRAPHY_H
#define CRYPTOGRAPHY_H

#include <cstdint>

#define ROTL16(x,y) ((uint16_t)((((uint16_t)(x))<<((y)&15)) | (((uint16_t)(x))>>(16-((y)&15)))))
#define ROTR16(x,y) ((uint16_t)((((uint16_t)(x))>>((y)&15)) | (((uint16_t)(x))<<(16-((y)&15)))))
#define ROTL32(x,y) ((uint32_t)((((uint32_t)(x))<<((y)&31)) | (((uint32_t)(x))>>(32-((y)&31)))))
#define ROTR32(x,y) ((uint32_t)((((uint32_t)(x))>>((y)&31)) | (((uint32_t)(x))<<(32-((y)&31)))))

namespace Common
{
	namespace Cryptography
	{
		struct Crypt
		{
			uint32_t RC5S[26];
			uint32_t RC6S[84];
			uint32_t UserKey = 0;
			bool isUsed = false;

			Crypt(uint32_t key = 0);

			void RC5KeySetup();

			void RC6KeySetup();

			void KeySetup(uint32_t key = 0);

			void RC5Encrypt32(const void* source, void* destination, int size);

			void RC5Decrypt32(const void* source, void* destination, int size);

			void RC5Encrypt64(const void* source, void* destination, int size);

			void RC5Decrypt64(const void* source, void* destination, int size);

			void RC6Encrypt128(const void* source, void* destination, int size);

			void RC6Decrypt128(const void* source, void* destination, int size);
		};
	}
}

#endif