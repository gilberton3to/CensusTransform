#cmake_minimum_required(VERSION 3.10)
#project(CensusTransform)
#
#set(CMAKE_CXX_STANDARD 20)
#
#find_package(OpenCV REQUIRED)
#
#add_executable(CensusTransform main.cpp
#        main.c)
#
#target_link_libraries(CensusTransform ${OpenCV_LIBS})
#target_include_directories(CensusTransform PRIVATE ${OpenCV_INCLUDE_DIRS})

cmake_minimum_required(VERSION 3.20)
project(SEMB C)

set(CMAKE_C_STANDARD 99)

add_executable(SEMB census_transform.c)