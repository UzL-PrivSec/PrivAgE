#include "SecSumAPI.h"

std::vector<long> cryptoIntVecToLong(std::vector<CryptoPP::Integer> v)
{
    std::vector<long> newVec;
    for (std::vector<CryptoPP::Integer>::iterator i = v.begin(); i != v.end(); ++i)
    {
        newVec.push_back(i->ConvertToLong());
    }
    return newVec;
}

/*
STEP 1
*/

std::pair<int, int> degreeAndThreshold(int n, double gamma, double delta, int sigma, int eta)
{
    HyperGeometricHelper hg;
    std::pair<int, int> ktTuple = hg.computeDegreeAndThreshold(n, gamma, delta, sigma, eta);
    return ktTuple;
}

std::map<std::string, std::vector<std::string>> getHararyNeighborMap(int n, int k, float gamma, float delta, float sigma, float eta)
{
    HararyHelper harary(n, k);
    harary.generateHarary();
    harary.randomShuffleVector(harary.nVector);
    harary.generateGMap();

    return harary.neighboursMap;
}

/*
STEP 8
*/

std::vector<long> reconstructSecSharing(int t, int l, std::vector<std::string> hbs)
{
    Base64Helper base64;
    ShamirSecretSharingHelper shamir;
    PrgHelper prng;

    std::vector<std::string> decHbs;
    for (std::string sh : hbs) {
        decHbs.push_back(base64.decodeString(sh));
    }

    CryptoPP::SecByteBlock recoveredRByteBlock;
    try {
        std::string recoveredBBase64 = shamir.reconstructSecret(t, decHbs);
        CryptoPP::SecByteBlock recoveredBByteBlock = base64.base64StringToSecByteBlock(recoveredBBase64);
        recoveredRByteBlock = prng.generateRandomFromSeed(recoveredBByteBlock, l);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    std::vector<CryptoPP::Integer> recoveredRVector = prng.transformByteBlockToIntegerVector(recoveredRByteBlock);

    return cryptoIntVecToLong(recoveredRVector);
}

std::vector<long> recoverY(std::string clientStep5Msg)
{
    PrgHelper prng;

    std::vector<CryptoPP::Integer> step5Vector = prng.deserializeIntVector(clientStep5Msg);

    return cryptoIntVecToLong(step5Vector);
}

std::vector<long> recoverDropoutInputs(int t, int l, std::vector<std::string> A1, std::vector<std::string> ngList, std::string pk1_str, std::string client, std::vector<std::string> step7Hs)
{
    PrgHelper prng;
    ShamirSecretSharingHelper shamir;
    Base64Helper base64;
    EcDiffieHellmanHelper ecDiffieHellmanHelper;
    Sha2Helper hash;

    std::vector<CryptoPP::Integer> sumMVector = prng.initializeNullVector(l);

    std::string recoveredSk1Base64 = shamir.reconstructSecret(t, step7Hs);
    CryptoPP::SecByteBlock recoveredSk1ByteBlock = base64.base64StringToSecByteBlock(recoveredSk1Base64);

    for (std::string iterator4 : A1)
    {
        if (client != iterator4)
        {
            if (std::find(ngList.begin(), ngList.end(), iterator4) != ngList.end())
            {
                CryptoPP::SecByteBlock pk1 = base64.base64StringToSecByteBlock(pk1_str);
                CryptoPP::SecByteBlock sharedSecret = ecDiffieHellmanHelper.generateSharedSecret(recoveredSk1ByteBlock, pk1);
                std::string sharedSecretString(reinterpret_cast<const char *>(&sharedSecret[0]), sharedSecret.size());
                CryptoPP::SecByteBlock s = hash.hashMessage(sharedSecretString);
                CryptoPP::SecByteBlock mByteBlock = prng.generateRandomFromSeed(s, l);
                std::vector<CryptoPP::Integer> mVector = prng.transformByteBlockToIntegerVector(mByteBlock);
                if (std::stoi(iterator4) < std::stoi(client))
                {
                    mVector = prng.negateVector(mVector);
                }
                sumMVector = prng.addTwoIntegerVectors(mVector, sumMVector);
            }
        }
    }

    return cryptoIntVecToLong(sumMVector);
}
