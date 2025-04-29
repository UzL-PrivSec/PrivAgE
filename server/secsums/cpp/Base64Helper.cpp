#include "Base64Helper.h"


CryptoPP::SecByteBlock Base64Helper::base64StringToSecByteBlock(std::string base64String) {
    std::string decoded;

    CryptoPP::StringSource ss(base64String, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded)
        ) // Base64Decoder
    ); // StringSource
    CryptoPP::SecByteBlock key(reinterpret_cast<const CryptoPP::byte*>(&decoded[0]), decoded.size());
    return key;
}


std::string Base64Helper::decodeString(std::string encoded) {
    std::string decoded;

    CryptoPP::StringSource ss(encoded, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded)
        ) // Base64Encoder
    ); // StringSource

    return decoded;
}
