#include "HararyHelper.h"

HararyHelper::HararyHelper(int amountUsers, int amountNeighbours)
{
	n = amountUsers;
	k = amountNeighbours;
	std::vector<int> tempVector;
	for (int iterator = 0; iterator < n; iterator++)
	{
		tempVector.push_back(0);
		nVector.push_back(0);
	}

	for (int iterator = 0; iterator < n; iterator++)
	{
		hararyMap.insert({std::to_string(iterator), tempVector});
		nVector[iterator] = iterator;
	}
}


void HararyHelper::oneHararyMap()
{
	for (int i = 0; i < n; i++)
	{
		for (int a = 0; a < n; a++)
		{
			hararyMap[std::to_string(i)][a] = 1;
		}
	}
}


void HararyHelper::generateHarary()
{

	if (k == n)
	{
		oneHararyMap();
		return;
	}

	int halfK = k / 2;
	for (int i = 0; i < n; i++)
	{
		for (int j = 1; j <= halfK; j++)
		{
			int nextNode = i + j;
			if (nextNode >= n)
			{
				nextNode = nextNode - n;
			}
			int prevNode = i - j;
			if (prevNode < 0)
			{
				prevNode = prevNode + n;
			}
			hararyMap[std::to_string(i)][nextNode] = 1;
			hararyMap[std::to_string(i)][prevNode] = 1;
		}
	}
	if (k % 2 == 1)
	{
		hararyOdd();
	}
}


void HararyHelper::hararyOdd()
{
	int halfDist;
	if (n % 2 == 0)
	{
		halfDist = n / 2;
	}
	else
	{
		halfDist = (n + 1) / 2;
	}
	for (int i = 0; i < n; i++)
	{
		int nextNode = i + halfDist;
		if (nextNode >= n)
		{
			nextNode = nextNode - n;
		}
		hararyMap[std::to_string(i)][nextNode] = 1;
	}
}


void HararyHelper::randomShuffleVector(std::vector<int> toShuffleVector)
{
	std::vector<int> tempVector = toShuffleVector;
	CryptoPP::AutoSeededRandomPool prng;
	prng.Shuffle(tempVector.begin(), tempVector.end());

	nVector = tempVector;
}

void HararyHelper::generateGMap()
{
	std::map<std::string, std::vector<int>> gMap;
	std::vector<int> tempVector = zeroVector();
	for (int i = 0; i < n; i++)
	{
		gMap.insert({std::to_string(i), tempVector});
	}

	for (int iterator = 0; iterator < n; iterator++)
	{
		int i = iterator;
		int piI = getIndex(nVector, i);
		for (int iterator2 = 0; iterator2 < n; iterator2++)
		{
			int j = iterator2;
			int piJ = getIndex(nVector, j);
			int result = hararyMap[std::to_string(piI)][piJ];
			gMap[std::to_string(i)][j] = result;
		}
	}
	std::map<std::string, std::vector<std::string>> ngMap;
	std::vector<std::string> emptyStringVector;
	for (int i = 0; i < n; i++)
	{
		ngMap.insert({std::to_string(i), emptyStringVector});
	}
	for (int i = 0; i < n; i++)
	{
		std::vector<int> gRow = gMap[std::to_string(i)];
		for (int a = 0; a < n; a++)
		{
			if (gRow[a] == 1)
			{
				ngMap[std::to_string(i)].push_back(std::to_string(a));
			}
		}
	}
	neighboursMap = ngMap;
}


int HararyHelper::getIndex(std::vector<int> v, int K)

{
	auto it = find(v.begin(), v.end(), K);

	// If element was found
	if (it != v.end())
	{

		// calculating the index
		// of K
		int index = it - v.begin();
		return index;
	}
	else
	{
		// If the element is not
		// present in the vector
		return -1;
	}
}


std::vector<int> HararyHelper::zeroVector()
{
	std::vector<int> tempVector;
	for (int i = 0; i < n; i++)
	{
		tempVector.push_back(0);
	}
	return tempVector;
}

