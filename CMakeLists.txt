cmake_minimum_required(VERSION 3.15)
project(tuntap4go C)

set(CMAKE_C_STANDARD 11)

set(source_files tuntap4go.c tuntap4go.h)
add_library(tuntap4go SHARED ${source_files})
add_library(tuntap4goStatic STATIC ${source_files})
set_target_properties(tuntap4goStatic PROPERTIES OUTPUT_NAME tuntap4go)
