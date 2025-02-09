FROM ubuntu:24.04 AS base

RUN apt update -y && \
    apt install -y \
    git \
    wget \
    g++ \
    cmake \
    ninja-build \
    libssl-dev

# Install boost
WORKDIR /root
RUN wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz && \
    tar -xf boost_1_87_0.tar.gz
WORKDIR /root/boost_1_87_0
RUN ./bootstrap.sh --with-libraries=charconv,system && \
    ./b2 install

FROM base AS build

WORKDIR /app
COPY . .

RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPIRE_BUILD_TESTS=OFF && \
    cmake --build build --config Release --target server ping

ENTRYPOINT ["./build/server"]