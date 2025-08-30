cmake_minimum_required(VERSION 3.10)
project(zketch_v12 CXX)

# Standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable RC language (resource compiler)
enable_language(RC)

# Sources
file(GLOB SOURCES "src/*.cpp")

# Resource file (RC) yang sudah include manifest
set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/version.rc")

# Buat executable
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${RESOURCE_FILE}    # ini penting: menyuntik manifest
)

# Include paths
target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/msys64/mingw64/include"
    "C:/msys64/mingw64/include/c++/15.2.0"
    "C:/msys64/mingw64/include/c++/15.2.0/x86_64-w64-mingw32"
)

# Compile flags
target_compile_options(${PROJECT_NAME} PRIVATE 
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -O3
    -march=native
    -DNDEBUG
    -mwindows
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    gdiplus
    gdi32
    user32
    ole32 
    oleaut32
)

# Output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")

message(STATUS "Build selesai! Eksekutable ada di folder 'bin/${PROJECT_NAME}.exe'")