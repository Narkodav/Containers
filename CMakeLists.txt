cmake_minimum_required(VERSION 3.20)
project(Containers LANGUAGES CXX)

# Add executable
add_library(${PROJECT_NAME} INTERFACE)

add_library(Containers::Containers ALIAS ${PROJECT_NAME})

# Require C++20
target_compile_features(${PROJECT_NAME}
    INTERFACE cxx_std_20
)

# Include directories
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Install
install(TARGETS ${PROJECT_NAME}
    EXPORT ContainersTargets
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT ContainersTargets
    FILE ContainersTargets.cmake
    NAMESPACE Containers::
    DESTINATION lib/cmake/Containers
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ContainersConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ContainersConfig.cmake
    INSTALL_DESTINATION lib/cmake/Containers
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ContainersConfig.cmake
    DESTINATION lib/cmake/Containers
)

# Add test executable
add_executable(ContainersTests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/BuddyAllocatorTests.cpp
)

target_link_libraries(ContainersTests PRIVATE ${PROJECT_NAME})

target_compile_options(ContainersTests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)