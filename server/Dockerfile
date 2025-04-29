FROM ubuntu

# create dedicated directory for our program.
RUN mkdir -p /server \
	&& useradd server
WORKDIR /server

# Install the dependencies for the python code base.
RUN apt-get update \
	&& apt-get install -y python3 python3-pip python3-venv \
# Install the dependencies for PostgreSQL.
	&& apt-get install -y postgresql libpq-dev \
# Install the dependencies for the c++ code.
	&& apt-get install -y swig make libboost1.74-all-dev libcrypto++-dev screen nano
RUN python3 -m venv server_env	
	
RUN cp /lib/x86_64-linux-gnu/libcryptopp.so /usr/lib/libcryptopp.so

# Preparing the server code.
RUN cp /usr/include/boost/beast/core/detail/base64.hpp /usr/include/base64.h

USER root

# Continue to install django as well as the adapter for PostgreSQL.
COPY . .

# Compiling the custom python module.
WORKDIR /server/secsums/cpp/swig/
RUN swig -c++ -python secsum_api.i
RUN python3 setup.py build_ext --inplace

WORKDIR /server/
RUN . server_env/bin/activate \
	&& pip install -r requirements.txt \
	&& deactivate

# Get it running.
WORKDIR /server
ENTRYPOINT ["/bin/bash", "entrypoint.sh"]
