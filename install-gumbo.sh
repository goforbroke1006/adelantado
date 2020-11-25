#!/usr/bin/env bash

set -e

# install Gumbo html parser https://github.com/google/gumbo-parser
if ldconfig -p | grep gumbo; then
  echo "GUMBO already installed!"
else
  sudo apt-get install -y libtool
  git clone -b v1.0.0 --depth 1 https://github.com/google/gumbo-parser.git
  (
    cd gumbo-parser || exit 1
    ./autogen.sh
    ./configure
    make
    sudo make install
    sudo ldconfig

    pkg-config --cflags gumbo        # print compiler flags
    pkg-config --libs gumbo          # print linker flags
    pkg-config --cflags --libs gumbo # print both
  )
  sudo rm -rf ./gumbo-parser
fi
