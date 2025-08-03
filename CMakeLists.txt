cmake_minimum_required(VERSION 4.0)
project(dns_proxy_filter_p2B9agE1 CXX)

# Compiler settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -pthread")

# Include directory
include_directories(include)

# Source files
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.cppm")

# Output executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
