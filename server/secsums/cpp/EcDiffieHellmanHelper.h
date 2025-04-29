#include <iostream>
#include <iomanip>
#include <utility>

#include "eccrypto.h"
#include "osrng.h"
#include "oids.h"

class EcDiffieHellmanHelper
{
public:
	CryptoPP::SecByteBlock generateSharedSecret(CryptoPP::SecByteBlock privKey, CryptoPP::SecByteBlock pubKey);
};
