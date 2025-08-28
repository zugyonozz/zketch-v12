cmake_minimum_required(VERSION 3.15)
project(zketch_v12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Bangun Semuanya Menjadi Satu Executable ---

add_executable(zketch_app "${PROJECT_SOURCE_DIR}/src/main.cpp")

# Beritahu executable di mana menemukan file-file header-nya.
target_include_directories(zketch_app PUBLIC "${PROJECT_SOURCE_DIR}/include")

# Link semua library Windows yang dibutuhkan.
target_link_libraries(zketch_app PUBLIC gdiplus user32 gdi32)