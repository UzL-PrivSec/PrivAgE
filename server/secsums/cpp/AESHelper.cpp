#include "AESHelper.h"

CryptoPP::SecByteBlock AESHelper::generateAesKey()
{
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    prng.GenerateBlock(key, key.size());
    return key;
}

CryptoPP::SecByteBlock AESHelper::generateAesIv()
{
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
    prng.GenerateBlock(iv, iv.size());
    return iv;
}

std::string AESHelper::encode(std::string plaintext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv)
{
    using namespace CryptoPP;

    HexEncoder encoder(new FileSink(std::cout));

    SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    key = aesKey;
    SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
    iv = aesIv;

    std::string plain = plaintext;
    std::string cipher, recovered;

    try
    {
        CBC_Mode<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(key, key.size(), iv);

        StringSource s(plain, true,
                       new StreamTransformationFilter(e,
                                                      new StringSink(cipher)) // StreamTransformationFilter
        );                                                                    // StringSource
    }
    catch (const Exception &e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    /*********************************\
    \*********************************/

    encoder.Put(key, key.size());
    encoder.MessageEnd();

    encoder.Put(iv, iv.size());
    encoder.MessageEnd();

    encoder.Put((const byte *)&cipher[0], cipher.size());
    encoder.MessageEnd();

    return cipher;

    /*********************************\
    \*********************************/
}

std::string AESHelper::decode(std::string ciphertext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv)
{
    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    key = aesKey;
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
    iv = aesIv;
    std::string cipher, recovered;
    cipher = ciphertext;
    try
    {
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption d;
        d.SetKeyWithIV(key, key.size(), iv);

        CryptoPP::StringSource s(cipher, true,
                                 new CryptoPP::StreamTransformationFilter(d,
                                                                          new CryptoPP::StringSink(recovered)) // StreamTransformationFilter
        );                                                                                                     // StringSource
    }
    catch (const CryptoPP::Exception &e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    return recovered;
}
