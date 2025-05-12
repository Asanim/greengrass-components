cmake_minimum_required(VERSION 3.1)
project (GreengrassComponents)

option(TRACE OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++17 -lstdc++fs")  

add_subdirectory(src/aws-iot-mqtt-publish)
add_subdirectory(src/aws-iot-shadow)
add_subdirectory(src/aws-iot-subscribe)

# A dummy target to build all examples in this directory
add_custom_target(examples)
add_dependencies(examples minimal lstpu classify)
