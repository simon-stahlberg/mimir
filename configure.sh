#!/bin/bash

use_conan=false
cmake_build_folder=build
cmake_source_folder=.
downstream_args=()
# Loop through all arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
  "--conan")
    use_conan=true
    ;;
  "--output="*)
    cmake_build_folder="${1#*=}"
    ;;
  "--output")
    cmake_build_folder="$2"
    shift # Move to the next argument
    ;;
  "--source="*)
    cmake_source_folder="${1#*=}"
    ;;
  "--source")
    cmake_source_folder="$2"
    shift # Move to the next argument
    ;;
  *)
    # If the argument doesn't match any known options, add it to downstream_args
    downstream_args+=("$1")
    ;;
  esac
  shift # Move to the next argument

done

script_dir=$(dirname $0)

echo "Configuration script called with the args..."
echo "use_conan: $use_conan"
echo "cmake_build_folder: $cmake_build_folder"
echo "cmake_source_folder: $cmake_source_folder"
echo "downstream_args: ${downstream_args[*]}"
echo "Executing cmake configuration."

if [ "$use_conan" = true ]; then
  conan install . -of="$cmake_build_folder/conan" --profile:host=default --profile:build=default --build=missing -g CMakeDeps
  cmake \
    -S "$cmake_source_folder" \
    -B "$cmake_build_folder" \
    -G Ninja \
    -DUSE_CONAN=$use_conan \
    -DCMAKE_TOOLCHAIN_FILE="$cmake_build_folder/conan/conan_toolchain.cmake" \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
    -DCMAKE_BUILD_TYPE=Release \
    "${downstream_args[*]}"
else
  cmake -S "$cmake_source_folder" -B "$cmake_build_folder" -G Ninja "${downstream_args[*]}"
fi
