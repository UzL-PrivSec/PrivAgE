
# Setup
Ubuntu 22.04. 
Python 3.10.12
GNU Make 4.3
C++ 11
SWIG 4.0.2
    Compiled with g++ [x86_64-pc-linux-gnu]
    Configured options: +pcre
boost  1.74.0.3
libcrypto++ 8.6.0

# Creating a Python Module from C++

- Install Python, C++ (gcc), Make, SWIG

- Cryptolib: https://www.cryptopp.com/wiki/Linux
- Boost: https://www.baeldung.com/linux/boost-install-on-ubuntu

- Create C++-Python Glue Code: swig -c++ -python ./secsum_api.i

- Compile Python Extension: python3 setup.py build_ext --inplace

# Getting Started

Use 'import secsum' in the Python Script

C++11 - Python3 Type Conversion
- Pair   -> Tuples
- Map    -> Dictionary
- Vector -> List