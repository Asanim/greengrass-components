
find_package(GreengrassIpc-cpp REQUIRED)

## Add ShadowExample executable
add_executable(shadow_example main.cc shadow_example.cc)
target_link_libraries(shadow_example AWS::GreengrassIpc-cpp)
