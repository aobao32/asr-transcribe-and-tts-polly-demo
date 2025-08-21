#!/bin/bash

#echo "Installing dependencies..."
#brew install portaudio pkg-config cmake git

# Clean up any existing AWS SDK
#rm -rf aws-sdk-cpp

# Build AWS SDK for C++ with static libraries
echo "Building AWS SDK for C++ with static libraries..."

# Download AWS SDK
#git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp

#cd aws-sdk-cpp
#rm -rf build
#mkdir build && cd build

#cmake .. -DCMAKE_BUILD_TYPE=Release \
#         -DBUILD_SHARED_LIBS=OFF \
#         -DBUILD_ONLY="transcribestreaming;core" \
#         -DCMAKE_INSTALL_PREFIX=/usr/local \
#         -DENABLE_TESTING=OFF

#make -j4
#if [ $? -eq 0 ]; then
#    sudo make install
#else
#    echo "AWS SDK build failed"
#    exit 1
#fi

# Return to project directory
#cd ../../

# Build the project
echo "Building project..."
rm -rf build
mkdir build && cd build

echo "Running cmake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    exit 1
fi

# Build transcribe_streaming
echo "Building transcribe_streaming..."
make transcribe_streaming

if [ $? -eq 0 ]; then
    echo "Build complete. Run with: ./transcribe_streaming"
else
    echo "Build failed"
    exit 1
fi