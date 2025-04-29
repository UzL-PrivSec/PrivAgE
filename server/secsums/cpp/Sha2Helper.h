#include <iostream>

#include "sha.h"
#include "secblock.h"

class Sha2Helper
{
public:
	CryptoPP::SecByteBlock hashMessage(std::string message);
};
