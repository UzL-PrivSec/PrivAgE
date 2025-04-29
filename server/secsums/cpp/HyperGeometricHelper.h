#include <iostream>
#include <cmath>
#include <utility>

#include <boost/math/policies/policy.hpp>
#include <boost/math/distributions/hypergeometric.hpp>

class HyperGeometricHelper
{
public:
	double computeCdfY(int n, int k, int t, double sigma);
	std::pair<int, int> computeDegreeAndThreshold(int n, double gamma, double delta, int sigma, int eta);
};
