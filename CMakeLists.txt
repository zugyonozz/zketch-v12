cmake_minimum_required(VERSION 3.15)
project(Win32TestProject VERSION 1.0 LANGUAGES CXX)

# Tentukan standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Folder include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Cari semua file cpp di src setiap kali build (lebih aman)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/test2.cpp"
)

# Buat executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Tentukan folder output bin
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

# Tambahkan library Win32
target_link_libraries(${PROJECT_NAME} PRIVATE
    user32
    gdi32
	gdiplus
	comctl32
)

# Opsional: tunjukkan semua perintah build (debugging)
set(CMAKE_VERBOSE_MAKEFILE ON)