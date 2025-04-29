#include <vector>
#include <sstream>
#include <ostream>
#include <iostream>

#include "cryptlib.h"
#include "secblock.h"
#include "rijndael.h"
#include "osrng.h"
#include "modes.h"
#include "hex.h"
#include "files.h"
#include "base64.h"

class PrgHelper
{
public:
	CryptoPP::SecByteBlock generateRandomFromSeed(CryptoPP::SecByteBlock seed, int l);
	std::vector<CryptoPP::Integer> transformByteBlockToIntegerVector(CryptoPP::SecByteBlock block);
	std::vector<CryptoPP::Integer> addTwoIntegerVectors(std::vector<CryptoPP::Integer> firstVector, std::vector<CryptoPP::Integer> secondVector);
	std::vector<CryptoPP::Integer> deserializeIntVector(std::string serializedVector);
	std::vector<CryptoPP::Integer> negateVector(std::vector<CryptoPP::Integer> inputVector);
	std::vector<CryptoPP::Integer> initializeNullVector(int sizeOfVector);

	std::list<std::string> splitDataString(std::string data, std::string delimiter);
};
