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

# Install vcpkg
WORKDIR /root
RUN git clone https://github.com/microsoft/vcpkg.git && \
    vcpkg/bootstrap-vcpkg.sh
ENV VCPKG_ROOT /root/vcpkg
ENV PATH /root/vcpkg:$PATH

# Setup
WORKDIR /app
COPY . .

RUN vcpkg install


FROM base AS build

# Build
RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPIRE_BUILD_TESTS=OFF && \
    cmake --build build --config Release --target server ping

ENTRYPOINT ["./build/server"]