
# The following is a quickfix as somewhere in cmake, CMAKE_MAJOR_VERSION CMAKE_MINOR_VERSION and CMAKE_PATCH_VERSION are set incorrectly causing the aws libraries to fail
# Print CMake version
message("src CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
message("src Full CMake version: ${CMAKE_VERSION}")

# Get major, minor, and patch versions from CMAKE_VERSION
string(REGEX MATCH "^[0-9]+" CMAKE_MAJOR_VERSION "${CMAKE_VERSION}")
string(REGEX MATCH "\\.[0-9]+\\." CMAKE_MINOR_VERSION "${CMAKE_VERSION}")
string(REGEX MATCH "\\.[0-9]+$" CMAKE_PATCH_VERSION "${CMAKE_VERSION}")

# Remove leading and trailing dots
string(REGEX REPLACE "\\." "" CMAKE_MINOR_VERSION "${CMAKE_MINOR_VERSION}")
string(REGEX REPLACE "\\." "" CMAKE_PATCH_VERSION "${CMAKE_PATCH_VERSION}")

# Print the extracted versions
message("CMAKE_VERSION: ${CMAKE_VERSION}")
message("CMAKE_MAJOR_VERSION: ${CMAKE_MAJOR_VERSION}")
message("CMAKE_MINOR_VERSION: ${CMAKE_MINOR_VERSION}")
message("CMAKE_PATCH_VERSION: ${CMAKE_PATCH_VERSION}")

## Set AWS include and library directories
set(AWS_SDK_DIR /opt/sdk_workspace_64)

list(APPEND CMAKE_PREFIX_PATH ${AWS_SDK_DIR}/lib/)
list(APPEND CMAKE_MODULE_PATH ${AWS_SDK_DIR}/lib/cmake/)


set(AWS_LIBRARY_DIR ${AWS_SDK_DIR}/lib/)
set(AWS_INCLUDE_DIR ${AWS_SDK_DIR}/include/)

find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)

## mqtt-publish executable
add_executable(mqtt-publish main.cc)

target_link_libraries(mqtt-publish AWS::GreengrassIpc-cpp)