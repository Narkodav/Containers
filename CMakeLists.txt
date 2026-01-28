cmake_minimum_required(VERSION 3.10)
project(Containers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
${CMAKE_SOURCE_DIR}/include
)