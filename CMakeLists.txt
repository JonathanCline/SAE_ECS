# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("SAE_ECS")

# Include sub-projects.
add_library(${PROJECT_NAME} STATIC "include/${PROJECT_NAME}.h" "source/${PROJECT_NAME}.cpp" "source/EntityFactory.h")
target_include_directories(${PROJECT_NAME} PUBLIC "include" PRIVATE "source")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME}build_test "tests/test.cpp")
target_link_libraries(${PROJECT_NAME}build_test PRIVATE ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}build_test PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED True)

