FROM ubuntu

# Versions of needed software.
ARG GRADLE_VERSION=8.10.1
ARG KOTLIN_VERSION=2.0.20
RUN mkdir -p /client \
	&& useradd client

# Installation of dependencies for gradle, kotlin and c++.
RUN apt-get update \
	&& apt-get install -y default-jdk default-jre wget unzip make libcrypto++-dev nano cmake g++ screen

# Gradle installation.
RUN wget https://services.gradle.org/distributions/gradle-${GRADLE_VERSION}-bin.zip -P /tmp \
	&& unzip -d /opt/gradle /tmp/gradle-${GRADLE_VERSION}-bin.zip
ENV PATH="${PATH}:/opt/gradle/gradle-${GRADLE_VERSION}/bin"

# Kotlin installation.
RUN wget https://github.com/JetBrains/kotlin/releases/download/v${KOTLIN_VERSION}/kotlin-compiler-${KOTLIN_VERSION}.zip -P /tmp \
	&& unzip -d /opt /tmp/kotlin-compiler-${KOTLIN_VERSION}.zip
ENV PATH="${PATH}:/opt/kotlinc/bin"

# Prepare the client code.
WORKDIR /client
COPY . .
RUN cp /lib/x86_64-linux-gnu/libcryptopp.so /usr/lib/libcryptopp.so
RUN cd src/main/cpp/secsum && cmake . && make
RUN cp src/main/cpp/secsum/libsecsum.so /usr/lib/libsecsum.so
RUN chmod +x ./start.sh

ENTRYPOINT ["/bin/bash"]
