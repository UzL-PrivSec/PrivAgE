#include "Sha2Helper.h"


CryptoPP::SecByteBlock Sha2Helper::hashMessage(std::string message) {
	CryptoPP::SecByteBlock digest(32 + 16);
	std::string msg = message;
	CryptoPP::SHA384 hash;
	hash.Update((const CryptoPP::byte*)msg.data(), msg.size());
	digest.resize(hash.DigestSize());
	hash.Final((CryptoPP::byte*)&digest[0]);

	return digest;

}
