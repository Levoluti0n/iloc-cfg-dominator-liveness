cmake_minimum_required(VERSION 3.16)
project(cfg_week2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cfg_week2
    src/main.cpp
)

target_include_directories(cfg_week2 PRIVATE src)