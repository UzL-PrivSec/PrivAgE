%module secsum

%include <std_pair.i>
%include <std_vector.i>
%include <std_string.i>
%include <std_map.i>

%template(IntPair) std::pair<double, double>;
%template(StringVector) std::vector<std::string>;
%template(LongVector) std::vector<long>;
%template(StringStringVectorMap) std::map<std::string, std::vector<std::string>>;

%{
#include "../SecSumAPI.h"
%}

extern std::pair<double, double> degreeAndThreshold(int n, double gamma, double delta, int sigma, int eta);
extern std::map<std::string, std::vector<std::string>> getHararyNeighborMap(int n, int k, float gamma, float delta, float sigma, float eta);

extern std::vector<long> reconstructSecSharing(int t, int l, std::vector<std::string> hbs);
extern std::vector<long> recoverY(std::string clientStep5Msg);
extern std::vector<long> recoverDropoutInputs(int t, int l, std::vector<std::string> A1, std::vector<std::string> ngList, std::string pk1, std::string client, std::vector<std::string> step7Hs);
