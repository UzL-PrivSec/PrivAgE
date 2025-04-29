#include <iostream>
#include <vector>

#include "osrng.h"
#include "channels.h"
#include "ida.h"

class ShamirSecretSharingHelper
{
public:
	std::string reconstructSecret(int thresh, std::vector<std::string> shares);
};
