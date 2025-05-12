find_package(GreengrassIpc-cpp REQUIRED)

## Add ShadowExample executable
add_executable(subscribe_example main.cc mqtt_example.cc subscription_example.cc)
target_link_libraries(subscribe_example AWS::GreengrassIpc-cpp)
