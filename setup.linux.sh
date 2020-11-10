#!/usr/bin/env bash

sudo apt install -y \
    libcurl4-openssl-dev

sudo apt-get install -y libgtest-dev
sudo apt-get install -y cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
# copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp lib/*.a /usr/lib
sudo ldconfig


sudo apt-get install -y libtool
git clone -b v1.0.0 --depth 1 https://github.com/google/gumbo-parser.git
(
  cd gumbo-parser
  ./autogen.sh
  ./configure
  make
  sudo make install
  sudo ldconfig

  pkg-config --cflags gumbo         # print compiler flags
  pkg-config --libs gumbo           # print linker flags
  pkg-config --cflags --libs gumbo  # print both
)
cd ..
sudo rm -rf ./gumbo-parser


# Postgres client
sudo apt install -y libpq-dev
pkg-config --libs libpq


sudo apt install -y libfmt-dev
pkg-config --libs libfmt-dev
