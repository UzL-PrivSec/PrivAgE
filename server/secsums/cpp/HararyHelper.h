#include <string>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <list>
#include <map>

#include "osrng.h"

class HararyHelper
{
public:
	std::map<std::string, std::vector<std::string>> neighboursMap;
        std::vector<int> nVector;

	HararyHelper(int amountUsers, int amountNeighbours);
	void generateHarary();
	void randomShuffleVector(std::vector<int> toShuffleVector);
	void generateGMap();
	
private:
	int n;
	int k;
	std::map<std::string, std::vector<int>> hararyMap;
	void oneHararyMap();
	void hararyOdd();
	int getIndex(std::vector<int> v, int K);
	std::vector<int> zeroVector();
};
