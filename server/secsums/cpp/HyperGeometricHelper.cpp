#include "HyperGeometricHelper.h"


double HyperGeometricHelper::computeCdfY(int n, int k, int t, double sigma)
{
    double cdfY;
    int oneSigmaN = (1 - sigma) * n;
    if (n - k <= oneSigmaN - t)
    {
        cdfY = 0;
    }
    else if (oneSigmaN <= t)
    {
        cdfY = 1.0;
    }
    else
    {
        boost::math::hypergeometric_distribution<double> hg_dist(oneSigmaN, k, n - 1);
        cdfY = boost::math::cdf<double>(hg_dist, t);
    }
    return cdfY;
}


// n = Number of nodes
// gamma  = Max. fraction of corrupt nodes
// delta  = Max. fraction of dropout nodes
// sigma  = statistical security parameter
// eta  = correctness parameter
// k = amount neigbours

std::pair<int, int> HyperGeometricHelper::computeDegreeAndThreshold(int n, double gamma, double delta, int sigma, int eta)
{
    int amountUsers = n;
    double fractionDropOutNodes = delta;

    // r(sucess objects), n(sample size), N(total amount of objects) is the order here
    double cdfX;
    double cdfY;

    for (int k = 1; k < n; k++)
    {
        boost::math::hypergeometric_distribution<double> hgDistX(gamma * n, k, n - 1);
        for (int t = 1; t < k; t++)
        {
            if (t - 1 >= gamma * n)
            {
                cdfX = 1.0;
            }
            else if (t - 1 < ((gamma * n) - (n - 1 - k)))
            {
                cdfX = 0;
            }
            else
            {

                cdfX = boost::math::cdf<double>(hgDistX, t - 1);
            }

            cdfY = computeCdfY(amountUsers, k, t, fractionDropOutNodes);

            double first = 1 - cdfX + (pow(double(delta + gamma), double(k / 2)));
            double second = cdfY;
            if (first < pow(2, -sigma) / n)
            {
                if (second < (pow(2, -eta) / n))
                {
                    auto pp = std::make_pair(k, t);

                    return pp;
                }
            }
        }
    }
}
