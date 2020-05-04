cmake_minimum_required(VERSION 3.5)
project(mercc)
file (GLOB SOURCES "source/*.cpp")
add_executable(mercc ${SOURCES})

target_include_directories(mercc
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)  