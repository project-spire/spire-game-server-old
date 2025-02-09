FROM ubuntu:24.04 AS base

RUN apt-get update -y && \
    apt-get install -y \
    git \
    wget curl zip unzip tar \
    g++ \
    cmake \
    ninja-build \
    libssl-dev \
    pkg-config

# Install boost (manually because installation via vcpkg is slow)
WORKDIR /root
RUN wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz && \
    tar -xf boost_1_87_0.tar.gz
WORKDIR /root/boost_1_87_0
RUN ./bootstrap.sh --with-libraries=system && \
    ./b2 install

# Install vcpkg
WORKDIR /root
RUN git clone https://github.com/microsoft/vcpkg.git && \
    vcpkg/bootstrap-vcpkg.sh
ENV VCPKG_ROOT /root/vcpkg
ENV PATH /root/vcpkg:$PATH


FROM base AS build

WORKDIR /app
COPY . .

RUN vcpkg install --clean-after-build
RUN cmake --preset release && \
    cmake --build build --target server ping

ENTRYPOINT ["./build/server"]