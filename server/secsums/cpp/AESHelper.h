#include <iostream>

#include "secblock.h"
#include "rijndael.h"
#include "osrng.h"
#include "modes.h"
#include "hex.h"
#include "files.h"

class AESHelper
{
public:
	CryptoPP::SecByteBlock generateAesKey();
	CryptoPP::SecByteBlock generateAesIv();
	std::string encode(std::string plaintext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv);
	std::string decode(std::string ciphertext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv);
};
