cmake_minimum_required(VERSION 4.1)
project(Scheduler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Scheduler
        Scheduler/include/Scheduler.h
        Scheduler/src/Scheduler.cpp
        Scheduler/main.cpp)

target_include_directories(Scheduler PUBLIC Scheduler/include)