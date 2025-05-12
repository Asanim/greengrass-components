#!/bin/bash

# Exit on any error
set -e

SCRIPT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
INSTALL_DIR="/opt/sdk_workspace_64/"
BUILD_PATH=${SCRIPT_PATH}/../build/sdk-workspace

rm -rf ${BUILD_PATH}
mkdir -p ${BUILD_PATH}

cd ${BUILD_PATH}

# Clone the AWS IoT Device SDK repository
git clone --recursive https://github.com/aws/aws-iot-device-sdk-cpp-v2.git

# Ensure all submodules are properly updated
cd aws-iot-device-sdk-cpp-v2
git submodule update --init --recursive
cd ..

# Make a build directory for the SDK. Can use any name.
# If working with multiple SDKs, using a SDK-specific name is helpful.
rm -rf aws-iot-device-sdk-cpp-v2-build
mkdir aws-iot-device-sdk-cpp-v2-build
cd aws-iot-device-sdk-cpp-v2-build

echo "$PWD"

# Configure the build, including the path to s2n
cmake ../aws-iot-device-sdk-cpp-v2 \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON
#    -DS2N_LIBRARY_PATH="/usr/local/lib/libs2n.so"  # Adjust the path as needed

# Build and install the AWS IoT Device SDK
cmake --build . --target install

# Clean up temporary build files
rm -rf ${BUILD_PATH}

echo "AWS IoT Device SDK C++ version 2 and s2n have been installed to $INSTALL_DIR"
