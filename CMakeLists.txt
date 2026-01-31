set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "" FORCE)
cmake_minimum_required(VERSION 3.10)
project(Containers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/**/*.cpp
)

# Add executable
add_library(${PROJECT_NAME} INTERFACE)

# Require C++20
target_compile_features(${PROJECT_NAME}
    INTERFACE cxx_std_20
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} INTERFACE
        -Wall -Wextra -Wpedantic -Werror
    )
elseif (MSVC)
    target_compile_options(${PROJECT_NAME} INTERFACE
        /W4 /WX
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Optional warnings for consumers (usually OFF by default)
option(CONTAINERS_ENABLE_WARNINGS "Enable warnings for Containers users" OFF)

if (CONTAINERS_ENABLE_WARNINGS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} INTERFACE
            -Wall -Wextra -Wpedantic
        )
    elseif (MSVC)
        target_compile_options(${PROJECT_NAME} INTERFACE
            /W4
        )
    endif()
endif()

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
    ${CMAKE_SOURCE_DIR}/install/ContainersConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ContainersConfig.cmake
    INSTALL_DESTINATION lib/cmake/Containers
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ContainersConfig.cmake
    DESTINATION lib/cmake/Containers
)

# Add test executable
add_executable(ContainersTests
    ${CMAKE_SOURCE_DIR}/Tests/main.cpp
)

target_link_libraries(ContainersTests PRIVATE ${PROJECT_NAME})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ContainersTests PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(ContainersTests PRIVATE /W4)
endif()