#!/usr/bin/env bash

set -e

# install https://github.com/nlohmann/json
if [[ ! -f modules/nlohmann-json/json.hpp ]]; then
  mkdir -p modules/nlohmann-json/
  curl https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp \
    -o modules/nlohmann-json/json.hpp
fi

#install CURL dev
sudo apt install -y \
  libcurl4-openssl-dev

# Postgres client
sudo apt install -y libpq-dev
pkg-config --libs libpq

#sudo apt install -y libfmt-dev
#pkg-config --libs libfmt-dev

#wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.11.tar.gz
#tar -xvzf libiconv-1.11.tar.gz
#(
#  cd libiconv-1.11 || exit 1
#  ./configure --prefix=/usr/local
#  make
#  sudo make install
#  sudo ldconfig
#)
#sudo rm -rf libiconv-1.11/
#rm -f libiconv-1.11.tar.gz

sudo apt install -y libconfig++-dev
pkg-config --libs libconfig++

if [[ ! -f adelantado.cfg ]] && [[ -f adelantado.cfg.dist ]]; then
  cp adelantado.cfg.dist adelantado.cfg
fi
