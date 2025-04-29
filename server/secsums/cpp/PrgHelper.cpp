#include "PrgHelper.h"

// Seed should be size 32+16 -> key + iv
CryptoPP::SecByteBlock PrgHelper::generateRandomFromSeed(CryptoPP::SecByteBlock seed, int l)
{
    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption prng;
    prng.SetKeyWithIV(seed, 32, seed + 32, 16);

    CryptoPP::SecByteBlock t(l);
    prng.GenerateBlock(t, t.size());

    CryptoPP::Integer x;
    x.Decode(t.BytePtr(), t.SizeInBytes());

    std::string s;
    CryptoPP::Base64Encoder base64(new CryptoPP::StringSink(s));

    base64.Put(t, t.size());
    base64.MessageEnd();

    return t;
}

std::vector<CryptoPP::Integer> PrgHelper::transformByteBlockToIntegerVector(CryptoPP::SecByteBlock block)
{
    std::vector<CryptoPP::byte> vector;
    std::vector<CryptoPP::Integer> intVector;
    vector.resize(block.size()); // Make room for elements
    std::memcpy(&vector[0], &block[0], vector.size());
    for (CryptoPP::byte temp : vector)
    {
        CryptoPP::Integer test(temp);
        intVector.push_back(test);
    }
    return intVector;
}

std::vector<CryptoPP::Integer> PrgHelper::addTwoIntegerVectors(std::vector<CryptoPP::Integer> firstVector, std::vector<CryptoPP::Integer> secondVector)
{
    std::vector<CryptoPP::Integer> resultVector;
    for (int i = 0; i < firstVector.size(); i++)
    {
        CryptoPP::Integer x;
        x = firstVector[i] + secondVector[i];
        resultVector.push_back(x);
    }

    return resultVector;
}


std::list<std::string> PrgHelper::splitDataString(std::string data, std::string delimiter)
{
    std::string s = data;
    std::list<std::string> splitData;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        splitData.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    splitData.push_back(s);
    return splitData;
}


std::vector<CryptoPP::Integer> PrgHelper::deserializeIntVector(std::string serializedVector)
{

    std::vector<CryptoPP::Integer> deserializedVector;

    std::list<std::string> tempList = splitDataString(serializedVector, ";");

    for (std::string iterator : tempList)
    {
        if (iterator != "")
        {
            int tempInt = std::stoi(iterator);

            CryptoPP::Integer tempInteger(tempInt);

            deserializedVector.push_back(tempInteger);
        }
    }

    return deserializedVector;
}

std::vector<CryptoPP::Integer> PrgHelper::negateVector(std::vector<CryptoPP::Integer> inputVector)
{
    std::vector<CryptoPP::Integer> resultVector;
    for (CryptoPP::Integer i : inputVector)
    {
        CryptoPP::Integer negator("-1");
        CryptoPP::Integer negatedI = i * negator;
        resultVector.push_back(negatedI);
    }

    return resultVector;
}

std::vector<CryptoPP::Integer> PrgHelper::initializeNullVector(int sizeOfVector)
{
    std::vector<CryptoPP::Integer> resultVector;
    for (int i = 0; i < sizeOfVector; i++)
    {
        CryptoPP::Integer temp("0");
        resultVector.push_back(temp);
    }
    return resultVector;
}
