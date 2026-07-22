#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-multimedia-dev \
  qml6-module-qtquick-dialogs

cmake --preset dev
cmake --build --preset dev
ctest --preset dev
