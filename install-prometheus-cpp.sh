#!/usr/bin/env bash

set -e

sudo apt install -y zlib1g-dev libcurl4-openssl-dev
sudo rm -rf prometheus-cpp/

git clone --depth 1 -b v0.11.0 https://github.com/jupp0r/prometheus-cpp.git

cd prometheus-cpp || exit 1
git submodule init
git submodule update

mkdir -p _build
cd _build || exit 1
# run cmake
cmake .. -DBUILD_SHARED_LIBS=ON # or OFF for static libraries

# build
make -j 4

# run tests
ctest -V

# install the libraries and headers
sudo make install
sudo ldconfig
