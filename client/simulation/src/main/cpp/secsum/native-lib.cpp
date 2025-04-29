#include <jni.h>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <iomanip>
#include <sstream>
#include <ostream>

#include "aes.h"
#include "secblock.h"
#include "cryptlib.h"
#include "rijndael.h"
#include "modes.h"
#include "files.h"
#include "osrng.h"
#include "hex.h"
#include "channels.h"
#include "ida.h"
#include "eccrypto.h"
#include "oids.h"
#include "sha.h"
#include "base64.h"


/*
 * Crypto Helper
 */


std::string secByteBlockToBase64String(CryptoPP::SecByteBlock secByteBlock) {
    std::string encoded;

    CryptoPP::Base64Encoder base64(new CryptoPP::StringSink(encoded));
    base64.Put(secByteBlock.BytePtr(), secByteBlock.SizeInBytes());
    base64.MessageEnd();

    return encoded;
}


CryptoPP::SecByteBlock base64StringToSecByteBlock(std::string base64String) {
    std::string decoded;

    CryptoPP::StringSource ss(base64String, true, new CryptoPP::Base64Decoder(
                                      new CryptoPP::StringSink(decoded)
                              )
    );
    CryptoPP::SecByteBlock key(reinterpret_cast<const CryptoPP::byte*>(&decoded[0]), decoded.size());

    return key;
}


std::string encodeString(std::string string) {
    std::string encoded;

    CryptoPP::StringSource ss(string, true,
                              new CryptoPP::Base64Encoder(
                                      new CryptoPP::StringSink(encoded)
                              )
    );

    return encoded;
}


std::string decodeString(std::string encoded) {
    std::string decoded;

    CryptoPP::StringSource ss(encoded, true,
                              new CryptoPP::Base64Decoder(
                                      new CryptoPP::StringSink(decoded)
                              )
    );

    return decoded;
}


std::tuple<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock> generateDhKeys() {
    CryptoPP::OID CURVE = CryptoPP::ASN1::secp256r1();
    CryptoPP::AutoSeededRandomPool rng;

    CryptoPP::ECDH < CryptoPP::ECP >::Domain dh(CURVE);
    CryptoPP::SecByteBlock priv(dh.PrivateKeyLength()), pub(dh.PublicKeyLength());
    dh.GenerateKeyPair(rng, priv, pub);

    return std::tuple<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock>{priv, pub};

}


CryptoPP::SecByteBlock generateSharedSecret(CryptoPP::SecByteBlock privKey, CryptoPP::SecByteBlock pubKey) {
    CryptoPP::OID CURVE = CryptoPP::ASN1::secp256r1();
    CryptoPP::ECDH < CryptoPP::ECP >::Domain dh(CURVE);
    CryptoPP::SecByteBlock sharedSecret(dh.AgreedValueLength());
    dh.Agree(sharedSecret, privKey, pubKey);

    return sharedSecret;

}


std::vector<std::string> generateShares(std::string secret, int thresh, int amountShares)
{
    const unsigned int CHID_LENGTH = 4;

    CryptoPP::AutoSeededRandomPool rng;

    CryptoPP::ChannelSwitch* channelSwitch = NULLPTR;
    CryptoPP::StringSource source(
            secret, false,
            new CryptoPP::SecretSharing(
                    rng, thresh, amountShares, channelSwitch = new CryptoPP::ChannelSwitch)
    );

    std::vector<std::string> strShares(amountShares);
    CryptoPP::vector_member_ptrs<CryptoPP::StringSink> strSinks(amountShares);
    std::string channel;

    for (unsigned int i = 0; i < amountShares; i++)
    {
        strSinks[i].reset(new CryptoPP::StringSink(strShares[i]));
        channel = CryptoPP::WordToString<CryptoPP::word32>(i);
        strSinks[i]->Put((const CryptoPP::byte*)channel.data(), CHID_LENGTH);
        channelSwitch->AddRoute(channel, *strSinks[i], CryptoPP::DEFAULT_CHANNEL);
    }
    source.PumpAll();

    return strShares;
}


CryptoPP::SecByteBlock hashMessage(std::string message) {
    CryptoPP::SecByteBlock digest(32 + 16);
    CryptoPP::SHA384 hash;
    hash.Update((const CryptoPP::byte*)message.data(), message.size());
    digest.resize(hash.DigestSize());
    hash.Final((CryptoPP::byte*)&digest[0]);

    return digest;
}


std::string encode(std::string plaintext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv) {
    CryptoPP::HexEncoder encoder;

    std::string cipher;

    CryptoPP::CBC_Mode< CryptoPP::AES >::Encryption e;
    e.SetKeyWithIV(aesKey, aesKey.size(), aesIv);

    CryptoPP::StringSource s(plaintext, true,
                             new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(cipher))
    );

    encoder.Put(aesKey, aesKey.size());
    encoder.MessageEnd();

    encoder.Put(aesIv, aesIv.size());
    encoder.MessageEnd();

    encoder.Put((const CryptoPP::byte*)&cipher[0], cipher.size());
    encoder.MessageEnd();

    return cipher;
}


std::string decode(std::string ciphertext, CryptoPP::SecByteBlock aesKey, CryptoPP::SecByteBlock aesIv) {

    std::string recovered;

    CryptoPP::CBC_Mode< CryptoPP::AES >::Decryption d;
    d.SetKeyWithIV(aesKey, aesKey.size(), aesIv);

    CryptoPP::StringSource s(ciphertext, true,
                             new CryptoPP::StreamTransformationFilter(d,
                                                                      new CryptoPP::StringSink(recovered)
                             )
    );

    return recovered;
}


CryptoPP::SecByteBlock generateRandomFromSeed(CryptoPP::SecByteBlock seed, int l) {

    //Seed should be size 32+16 -> key + iv

    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption ctr_enc;

    ctr_enc.SetKeyWithIV(seed, 32, seed + 32, 16);

    CryptoPP::SecByteBlock t(l);

    ctr_enc.GenerateBlock(t, t.size());

    CryptoPP::Integer x;
    x.Decode(t.BytePtr(), t.SizeInBytes());

    std::string s;
    CryptoPP::Base64Encoder base64(new CryptoPP::StringSink(s));

    base64.Put(t, t.size());
    base64.MessageEnd();

    return t;
}


std::vector<CryptoPP::Integer> transformByteBlockToIntegerVector(CryptoPP::SecByteBlock block) {
    std::vector<CryptoPP::byte> vector;
    std::vector<CryptoPP::Integer> intVector;

    vector.resize(block.size());  // Make room for elements
    std::memcpy(&vector[0], &block[0], vector.size());
    for (CryptoPP::byte temp : vector) {
        CryptoPP::Integer test(temp);
        intVector.push_back(test);
    }

    return intVector;
}


std::vector<CryptoPP::Integer> addTwoIntegerVectors(std::vector<CryptoPP::Integer> firstVector, std::vector<CryptoPP::Integer> secondVector) {
    std::vector<CryptoPP::Integer> resultVector;

    for (int i = 0; i < firstVector.size(); i++) {
        CryptoPP::Integer x;
        x = firstVector[i] + secondVector[i];
        resultVector.push_back(x);
    }

    return resultVector;
}


std::string serialzeIntVector(std::vector <CryptoPP::Integer> intVector) {
    std::string result;

    for (CryptoPP::Integer i : intVector) {
        std::ostringstream oss;
        oss << i;
        result += oss.str();
        result += ";";
    }

    return result;
}


std::vector<CryptoPP::Integer> negateVector(std::vector <CryptoPP::Integer> inputVector) {
    std::vector<CryptoPP::Integer> resultVector;

    for (CryptoPP::Integer i : inputVector) {
        CryptoPP::Integer negator("-1");
        CryptoPP::Integer negatedI = i * negator;
        resultVector.push_back(negatedI);
    }

    return resultVector;
}


std::vector<CryptoPP::Integer> initializeNullVector(int sizeOfVector) {
    std::vector<CryptoPP::Integer> resultVector;

    for (int i = 0; i < sizeOfVector; i++) {
        CryptoPP::Integer temp("0");
        resultVector.push_back(temp);
    }

    return resultVector;
}


/*
 * Protocol Steps
 */


std::tuple<std::string, std::string, std::string, std::string> step2(){

    std::tuple<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock> ecKeyPair1 = generateDhKeys();
    std::tuple<CryptoPP::SecByteBlock, CryptoPP::SecByteBlock> ecKeyPair2 = generateDhKeys();

    std::string ecPublicKey1Base64 = secByteBlockToBase64String(std::get<1>(ecKeyPair1));
    std::string ecPrivateKey1Base64 = secByteBlockToBase64String(std::get<0>(ecKeyPair1));
    std::string ecPrivateKey2Base64 = secByteBlockToBase64String(std::get<0>(ecKeyPair2));
    std::string ecPublicKey2Base64 = secByteBlockToBase64String(std::get<1>(ecKeyPair2));

    return {ecPublicKey1Base64, ecPrivateKey1Base64, ecPublicKey2Base64, ecPrivateKey2Base64};
}


std::tuple<std::string, std::map<std::string, std::string>> step3(int threshold, std::string b64SecKey1, std::string b64SecKey2,
           std::string strClientId, std::map<std::string, std::string> strIdPk2Map){

    //Generate a PRG seed bi
    CryptoPP::SecByteBlock bByteBlock(32 + 16);
    CryptoPP::OS_GenerateRandomBlock(false, bByteBlock, bByteBlock.size());
    std::string b = secByteBlockToBase64String(bByteBlock);

    //Compute share Hb
    std::vector<std::string> tempShareHbVector = generateShares(b, threshold, strIdPk2Map.size());
    std::list<std::string> myBase64HbShares;
    for (std::string iterator : tempShareHbVector) {
        std::string encoded = encodeString(iterator);
        myBase64HbShares.push_front(encoded);
    }

    //Compute share Hs
    std::vector<std::string> tempShareHsVector = generateShares(b64SecKey1, threshold, strIdPk2Map.size());
    std::list<std::string> myBase64HsShares;
    for (std::string iterator : tempShareHsVector) {
        std::string encoded;
        encoded = encodeString(iterator);
        myBase64HsShares.push_front(encoded);
    }

    //Compute cij for each j in Ng
    std::map<std::string, std::string> ciphersById;
    for (auto it = strIdPk2Map.begin(); it != strIdPk2Map.end(); ++it) {
        std::string c;
        c = strClientId + ";" + it->first + ";" + myBase64HbShares.front() + ";" + myBase64HsShares.front();
        myBase64HbShares.pop_front();
        myBase64HsShares.pop_front();
        std::string cEncrypted;
        CryptoPP::SecByteBlock sk2ByteBlock = base64StringToSecByteBlock(b64SecKey2);
        CryptoPP::SecByteBlock pk2 = base64StringToSecByteBlock(strIdPk2Map[it->first]);
        CryptoPP::SecByteBlock sharedSecret2 = generateSharedSecret(sk2ByteBlock, pk2);
        std::string sharedSecretString2(reinterpret_cast<const char*>(&sharedSecret2[0]), sharedSecret2.size());
        CryptoPP::SecByteBlock k = hashMessage(sharedSecretString2);
        CryptoPP::SecByteBlock key(k.BytePtr(), 32);
        CryptoPP::SecByteBlock iv(k.BytePtr() + 32, 16);
        cEncrypted = encode(c, key, iv);
        std::string cEncryptedBase64 = encodeString(cEncrypted);
        ciphersById[it->first] = cEncryptedBase64;
    }

    return {b, ciphersById};
}


std::tuple<std::string> step5(std::list<int> inputList, int l, std::list<std::string> a1, std::map<std::string, std::string> strIdPk1Map,
           std::string strB64SecKey1, std::string strClientId, std::string bB64){

    std::vector<CryptoPP::Integer> xVector = initializeNullVector(l);
    int i = 0;
    for (int xInt : inputList) {
        xVector[i] = xInt;
        i++;
    }

    //Generate PRG Seed si,j = KA(sk1i, pk1j)
    //Using sha3-384 to generate a seed with the correct size of 32+16 -> key + iv for aes in ctr mode

    std::vector<CryptoPP::Integer> sumMVector = initializeNullVector(l);

    CryptoPP::SecByteBlock sk1ByteBlock = base64StringToSecByteBlock(strB64SecKey1);

    for (std::string iterator : a1) {
        CryptoPP::SecByteBlock pk1 = base64StringToSecByteBlock(strIdPk1Map[iterator]);

        CryptoPP::SecByteBlock sharedSecret = generateSharedSecret(sk1ByteBlock, pk1);

        std::string sharedSecretString(reinterpret_cast<const char*>(&sharedSecret[0]), sharedSecret.size());

        CryptoPP::SecByteBlock s = hashMessage(sharedSecretString);

        CryptoPP::SecByteBlock mByteBlock = generateRandomFromSeed(s, l);

        std::vector<CryptoPP::Integer> mVector = transformByteBlockToIntegerVector(mByteBlock);

        if (std::stoi(iterator) < std::stoi(strClientId)) {
            mVector = negateVector(mVector);

        }

        sumMVector = addTwoIntegerVectors(mVector, sumMVector);
    }

    CryptoPP::SecByteBlock bByteBlock = base64StringToSecByteBlock(bB64);
    CryptoPP::SecByteBlock rByteBlock = generateRandomFromSeed(bByteBlock, l);
    std::vector<CryptoPP::Integer> rVector = transformByteBlockToIntegerVector(rByteBlock);

    std::vector<CryptoPP::Integer> yVector = addTwoIntegerVectors(rVector, sumMVector);

    yVector = addTwoIntegerVectors(xVector, yVector);

    std::string yString = serialzeIntVector(yVector);

    return {yString};
}


std::tuple<std::vector<std::string>> step7(std::string strClientId, std::string strB64SecKey2,
           std::map<std::string, std::string> encodedMsgsMap, std::map<std::string, std::string> strIdPk2Map){

    std::vector<std::string> decodedCiphers;

    for (auto it = strIdPk2Map.begin(); it != strIdPk2Map.end(); ++it) {
        std::string encodedC = decodeString(encodedMsgsMap[it->first]);
        CryptoPP::SecByteBlock sk2ByteBlock = base64StringToSecByteBlock(strB64SecKey2);
        CryptoPP::SecByteBlock pk2 = base64StringToSecByteBlock(it->second);

        CryptoPP::SecByteBlock sharedSecret2 = generateSharedSecret(sk2ByteBlock, pk2);
        std::string sharedSecretString2(reinterpret_cast<const char*>(&sharedSecret2[0]), sharedSecret2.size());

        CryptoPP::SecByteBlock k = hashMessage(sharedSecretString2);
        CryptoPP::SecByteBlock key(k.BytePtr(), 32);
        CryptoPP::SecByteBlock iv(k.BytePtr() + 32, 16);

        std::string c = decode(encodedC, key, iv);

        decodedCiphers.push_back(c);
    }

    return {decodedCiphers};
}


/*
 * JNI Helper
 */


std::list<int> toIntList(JNIEnv* env, jintArray array) {
    std::list<int> list;

    jsize arrayLength = env->GetArrayLength(array);
    jint* elements = env->GetIntArrayElements(array, 0);
    for (jsize i = 0; i < arrayLength; ++i) {
        list.push_back(elements[i]);
    }

    env->ReleaseIntArrayElements(array, elements, 0);

    return list;
}


std::list<std::string> toStringList(JNIEnv* env, jobjectArray array) {
    std::list<std::string> list;

    jsize arrayLength = env->GetArrayLength(array);
    for (jsize i = 0; i < arrayLength; ++i) {
        jstring jStr = (jstring) env->GetObjectArrayElement(array, i);
        list.push_back(env->GetStringUTFChars(jStr, 0));
    }

    return list;
}


std::map<std::string, std::string> toStrStrMap(JNIEnv* env, jobject mapObject) {
    std::map<std::string, std::string> map;

    jclass mapClass = env->GetObjectClass(mapObject);
    jmethodID entrySetMethod = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    jobject entrySet = env->CallObjectMethod(mapObject, entrySetMethod);

    jclass setClass = env->FindClass("java/util/Set");
    jmethodID iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    jobject iterator = env->CallObjectMethod(entrySet, iteratorMethod);

    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    jclass entryClass = env->FindClass("java/util/Map$Entry");
    jmethodID getKeyMethod = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    jmethodID getValueMethod = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

    while (env->CallBooleanMethod(iterator, hasNextMethod)) {
        jobject entry = env->CallObjectMethod(iterator, nextMethod);

        jstring jStrKey = (jstring) env->CallObjectMethod(entry, getKeyMethod);
        jstring jStrValue = (jstring) env->CallObjectMethod(entry, getValueMethod);

        std::string key = env->GetStringUTFChars(jStrKey, nullptr);
        std::string value = env->GetStringUTFChars(jStrValue, nullptr);

        map[key] = value;

        env->DeleteLocalRef(entry);
        env->DeleteLocalRef(jStrKey);
        env->DeleteLocalRef(jStrValue);
    }

    env->DeleteLocalRef(entrySet);
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(setClass);
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(entryClass);

    return map;
}


/*
 * JNI Interface
 */


extern "C"
JNIEXPORT void JNICALL
Java_clientsim_SecureSummation_JNIstep2(
        JNIEnv* env,
        jobject thiz,
        jobject dataObj) {

	auto res = step2();

    auto ecPublicKey1Base64 = std::get<0>(res);
    auto ecPrivateKey1Base64 = std::get<1>(res);
    auto ecPublicKey2Base64 = std::get<2>(res);
    auto ecPrivateKey2Base64 = std::get<3>(res);

    jclass clazz = env->GetObjectClass(dataObj);

    jfieldID pubKey1Field = env->GetFieldID(clazz, "pubKey1", "Ljava/lang/String;");
    env->SetObjectField(dataObj, pubKey1Field, env->NewStringUTF(ecPublicKey1Base64.c_str()));

    jfieldID pubKey2Field = env->GetFieldID(clazz, "pubKey2", "Ljava/lang/String;");
    env->SetObjectField(dataObj, pubKey2Field, env->NewStringUTF(ecPublicKey2Base64.c_str()));

    jfieldID privKey1Field = env->GetFieldID(clazz, "privKey1", "Ljava/lang/String;");
    env->SetObjectField(dataObj, privKey1Field, env->NewStringUTF(ecPrivateKey1Base64.c_str()));

    jfieldID privKey2Field = env->GetFieldID(clazz, "privKey2", "Ljava/lang/String;");
    env->SetObjectField(dataObj, privKey2Field, env->NewStringUTF(ecPrivateKey2Base64.c_str()));
}

extern "C"
JNIEXPORT void JNICALL
Java_clientsim_SecureSummation_JNIstep3(
        JNIEnv* env,
        jobject thiz,
		jint threshold,
        jstring base64SecKey1,
        jstring base64SecKey2,
        jstring clientId,
        jobject idPk2Map,
        jobject dataObj) {

    std::string strB64SecKey1 = env->GetStringUTFChars(base64SecKey1, 0);
    std::string strB64SecKey2 = env->GetStringUTFChars(base64SecKey2, 0);
    std::string strClientId = env->GetStringUTFChars(clientId, 0);
	auto strIdPk2Map = toStrStrMap(env, idPk2Map);

	auto res = step3(threshold, strB64SecKey1, strB64SecKey2, strClientId, strIdPk2Map);

    auto bBase64 = std::get<0>(res);
    auto ciphersById = std::get<1>(res);

    jclass clazz = env->GetObjectClass(dataObj);

    jfieldID bBase64Field = env->GetFieldID(clazz, "bBase64", "Ljava/lang/String;");
    env->SetObjectField(dataObj, bBase64Field, env->NewStringUTF(bBase64.c_str()));

    jfieldID cipherMapField = env->GetFieldID(clazz, "cipherMap", "Ljava/util/HashMap;");
    jobject hashMap = env->GetObjectField(dataObj, cipherMapField);
    jclass clazzHashMap = env->GetObjectClass(hashMap);
    jmethodID putMethod = env->GetMethodID(clazzHashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    for (auto it = ciphersById.begin(); it != ciphersById.end(); ++it) {
        env->CallObjectMethod(hashMap, putMethod, env->NewStringUTF(it->first.c_str()), env->NewStringUTF(it->second.c_str()));
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_clientsim_SecureSummation_JNIstep5(
        JNIEnv* env,
        jobject thiz,
        jintArray inputIntList,
        jint l,
        jobjectArray a1,
        jobject idPk1Map,
        jstring myBase64SecretKey1,
        jstring clientId,
        jstring bBase64,
        jobject dataObj) {

    std::list<int> inputList = toIntList(env, inputIntList);

    std::list<std::string> strA1 = toStringList(env, a1);
    auto strIdPk1Map = toStrStrMap(env, idPk1Map);
    std::string strB64SecKey1 = env->GetStringUTFChars(myBase64SecretKey1, 0);
    std::string strClientId = env->GetStringUTFChars(clientId, 0);
    std::string bB64 = env->GetStringUTFChars(bBase64, 0);

	auto res = step5(inputList, l, strA1, strIdPk1Map, strB64SecKey1, strClientId, bB64);

    auto yVectorStr = std::get<0>(res);

    jclass clazz = env->GetObjectClass(dataObj);
    jfieldID yVecField = env->GetFieldID(clazz, "maskedInput", "Ljava/lang/String;");
    env->SetObjectField(dataObj, yVecField, env->NewStringUTF(yVectorStr.c_str()));
}

extern "C"
JNIEXPORT void JNICALL
Java_clientsim_SecureSummation_JNIstep7(
        JNIEnv* env,
        jobject thiz,
        jstring clientId,
        jstring myBase64SecretKey2,
        jobjectArray encodedMsgsMap,
        jobject idPk2Map,
        jobject dataObj) {

    std::string strClientId = env->GetStringUTFChars(clientId, 0);
    std::string strB64SecKey2 = env->GetStringUTFChars(myBase64SecretKey2, 0);
    auto strEncodedMsgsMap = toStrStrMap(env, encodedMsgsMap);
    auto strIdPk2Map = toStrStrMap(env, idPk2Map);

	auto res = step7(strClientId, strB64SecKey2, strEncodedMsgsMap, strIdPk2Map);

    auto decodedCiphers = std::get<0>(res);

    jclass clazz = env->GetObjectClass(dataObj);
    jfieldID decodedCiphersField = env->GetFieldID(clazz, "decodedCiphers", "Ljava/util/List;");
    jobject list = env->GetObjectField(dataObj, decodedCiphersField);
    jclass clazzList = env->GetObjectClass(list);
    jmethodID addMethod = env->GetMethodID(clazzList, "add", "(Ljava/lang/Object;)Z");
    for (auto cipher: decodedCiphers) {
        env->CallObjectMethod(list, addMethod, env->NewStringUTF(cipher.c_str()));
    }
}

