cmake_minimum_required(VERSION 3.15)
project(zketch_v12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include folder
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# Cari semua source
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Library
if(SRC_FILES)
    add_library(zketch STATIC ${SRC_FILES})
    target_include_directories(zketch PUBLIC ${INCLUDE_DIR})
    target_link_libraries(zketch PUBLIC gdiplus)
endif()

# Executable
if(EXISTS "${PROJECT_SOURCE_DIR}/src/main.cpp")
    add_executable(zketch_app ${PROJECT_SOURCE_DIR}/src/main.cpp)
    target_include_directories(zketch_app PRIVATE ${INCLUDE_DIR})
    target_link_libraries(zketch_app PRIVATE zketch gdiplus)
endif()
