
find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)

## mqtt-publish executable
add_executable(mqtt-publish main.cc)

target_link_libraries(mqtt-publish AWS::GreengrassIpc-cpp)