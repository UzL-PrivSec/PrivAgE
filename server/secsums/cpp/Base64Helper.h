#include <iostream>
#include <base64.h>

class Base64Helper
{
public:
	CryptoPP::SecByteBlock base64StringToSecByteBlock(std::string base64String);
	std::string decodeString(std::string encoded);
};
