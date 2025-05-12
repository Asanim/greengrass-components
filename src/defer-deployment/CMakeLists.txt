find_package(GreengrassIpc-cpp REQUIRED)

## Add ShadowExample executable
add_executable(DeferDeployment main.cc mqtt_example.cc component_update_example.cc)
target_link_libraries(DeferDeployment AWS::GreengrassIpc-cpp)
