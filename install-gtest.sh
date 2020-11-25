#!/usr/bin/env bash

set -e

# install Google Test
if pkg-config --libs gtest; then
  echo "GTEST already installed!"
else
  sudo apt-get install -y libgtest-dev
  sudo apt-get install -y cmake
  (
    cd /usr/src/gtest || exit 1
    sudo cmake CMakeLists.txt
    sudo make
    # copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
    ls -lah
    sudo cp ./*.a /usr/lib || true
    sudo cp lib/*.a /usr/lib || true
  )
  sudo ldconfig
fi
