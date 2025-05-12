
find_package(aws-crt-cpp REQUIRED)
find_package(EventstreamRpc-cpp REQUIRED)
find_package(GreengrassIpc-cpp REQUIRED)

## Add ShadowExample executable
add_executable(shadow_example main.cc shadow_example.cc)
target_link_libraries(shadow_example AWS::GreengrassIpc-cpp aws-crt-cpp EventstreamRpc-cpp)
