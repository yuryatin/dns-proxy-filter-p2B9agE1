cmake_minimum_required(VERSION 3.31)

project(dns_proxy_filter_p2B9agE1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API TRUE)

add_executable(${PROJECT_NAME} ${SRC_FILES})

if(APPLE)
    message(STATUS "Using Apple ******")
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fmodules -fcxx-modules
        -isysroot ${SDK_PATH}
    )
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall -Wextra -O2 -pthread
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang: enabling modules flags")
    add_compile_options(-fmodules -fcxx-modules)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC: enabling modules-ts flag")
    add_compile_options(-fmodules-ts)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC: no extra flags needed for modules")
endif()

# Include directory
include_directories(include)

# Source files
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp")

file(GLOB CPPM_FILES CONFIGURE_DEPENDS 
    "${CMAKE_SOURCE_DIR}/include/*.cppm")

target_sources(${PROJECT_NAME}
    PRIVATE ${SRC_FILES}
    PRIVATE FILE_SET modules TYPE CXX_MODULES FILES ${CPPM_FILES}
)

