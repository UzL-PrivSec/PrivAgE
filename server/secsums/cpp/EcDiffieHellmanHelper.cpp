#include "EcDiffieHellmanHelper.h"


CryptoPP::SecByteBlock EcDiffieHellmanHelper::generateSharedSecret(CryptoPP::SecByteBlock privKey, CryptoPP::SecByteBlock pubKey)
{
	CryptoPP::OID CURVE = CryptoPP::ASN1::secp256r1();
	CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CURVE);
	CryptoPP::SecByteBlock sharedSecret(dh.AgreedValueLength());
	dh.Agree(sharedSecret, privKey, pubKey);

	return sharedSecret;
}
