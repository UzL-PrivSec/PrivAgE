#!/usr/bin/env python

from setuptools import Extension, setup


secsum_module = Extension(
    name="_secsum",
    sources=[
        "../SecSumAPI.cpp",
        "secsum_api_wrap.cxx",
        "../HararyHelper.cpp",
        "../PrgHelper.cpp",
        "../EcDiffieHellmanHelper.cpp",
        "../HyperGeometricHelper.cpp",
        "../Base64Helper.cpp",
        "../ShamirSecretSharingHelper.cpp",
        "../Sha2Helper.cpp",
    ],
    include_dirs=["/usr/include/crypto++"],
    libraries=["cryptopp"],
    language="c++",
)

setup(
    name="secsum",
    version="0.1",
    author="Me",
    description="""Simple swig secsum example from docs""",
    ext_modules=[secsum_module],
    py_modules=["secsum"],
)
