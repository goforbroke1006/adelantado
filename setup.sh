#!/usr/bin/env bash

set -e

#install CURL dev
sudo apt install -y libcurl4-openssl-dev

# Postgres client
sudo apt install -y libpq-dev
pkg-config --libs libpq

sudo apt install -y libconfig++-dev
pkg-config --libs libconfig++

if [[ ! -f adelantado.cfg ]] && [[ -f adelantado.cfg.dist ]]; then
  cp adelantado.cfg.dist adelantado.cfg
fi
