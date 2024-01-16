#!/bin/bash

if [[ -n $1 ]]; then
  cmake_build_folder=$1
else
  cmake_build_folder=build
fi

cmake --build "$cmake_build_folder" --target all
