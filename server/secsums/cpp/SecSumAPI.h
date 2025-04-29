#include "HyperGeometricHelper.h"
#include "HararyHelper.h"
#include "ShamirSecretSharingHelper.h"
#include "Base64Helper.h"
#include "PrgHelper.h"
#include "EcDiffieHellmanHelper.h"
#include "Sha2Helper.h"

#include "secblock.h"
#include "rijndael.h"
#include "osrng.h"
#include "modes.h"
#include "hex.h"
#include "files.h"

#include <string>
#include <vector>
#include <map>
#include <utility>

std::vector<long> cryptoIntVecToLong(std::vector<CryptoPP::Integer> v);

/*
STEP 1
*/

std::pair<int, int> degreeAndThreshold(int n, double gamma, double delta, int sigma, int eta);

std::map<std::string, std::vector<std::string>> getHararyNeighborMap(int n, int k, float gamma, float delta, float sigma, float eta);

/*
STEP 8
*/

std::vector<long> reconstructSecSharing(int t, int l, std::vector<std::string> hbs);

std::vector<long> recoverY(std::string clientStep5Msg);

std::vector<long> recoverDropoutInputs(int t, int l, std::vector<std::string> A1, std::vector<std::string> ngList, std::string pk1_str, std::string client, std::vector<std::string> step7Hs);
