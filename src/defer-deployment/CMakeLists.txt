find_package(GreengrassIpc-cpp REQUIRED)

## Add ShadowExample executable
add_executable(DeferDeployment main_standalone.cc)
#  mqtt_example.cc subscription_example.cc)
target_link_libraries(DeferDeployment AWS::GreengrassIpc-cpp)
