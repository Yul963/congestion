cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(NAME congestion_calculator)
project(${NAME})

#set(CMAKE_SYSTEM_NAME Linux)
#set(CMAKE_SYSTEM_PROCESSOR arm)

#set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
#set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")

#option(USE_CUDNN "Use cuDNN" ON)

#디렉토리는 시스템마다 변경 필요

#set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda-11.8)
#set(CMAKE_PREFIX_PATH "/usr/local/cuda-11.8")
#set(CUDNN_LIBRARY "/usr/lib/x86_64-linux-gnu/libcudnn.so")
#set(CMAKE_CUDA_COMPILER /usr/local/cuda-11.8/bin/nvcc)
set(Torch_DIR /home/yul/libtorch/share/cmake/Torch)
#set(OpenCV_DIR /home/yul/opencv_arm/lib/cmake/opencv4)
#set(Boost_DIR /home/yul/boost)
set (BOOST_ROOT /home/yul/boost)
set (BOOST_LIBRARY_DIR /home/yul/boost/stage/lib)
set (BOOST_INCLUDE_DIR ${BOOST_ROOT})

#set(SQLITE_ROOT /home/yul/sqlite)
set(CMAKE_GENERATOR Ninja)

find_package(Torch 2.0.0 REQUIRED)
#find_package(CUDA REQUIRED)
#find_package(CUDNN REQUIRED)
find_package(OpenCV 4.5.1 REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.82.0 COMPONENTS system filesystem REQUIRED)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS})

include_directories(include)
#include_directories(${SQLITE_ROOT})
#include_directories(${CUDA_INCLUDE_DIRS})
#include_directories(${CUDNN_INCLUDE_DIRS})
include_directories(${BOOST_INCLUDE_DIR})

link_directories(${BOOST_LIBRARY_DIR})
#add_library(sqlite3 ${SQLITE_ROOT}/sqlite3.c)
add_executable(${NAME} main.cpp VideoProcessor.cpp ImageProcessor.cpp)#ClientHandler.cpp

#target_link_libraries(${NAME} ${CUDA_LIBRARIES} ${CUDNN_LIBRARIES})
target_link_libraries(${NAME} Threads::Threads)
target_link_libraries(${NAME} ${TORCH_LIBRARIES})
target_link_libraries(${NAME} ${OpenCV_LIBS})
#target_link_libraries(${NAME} sqlite3)
target_link_libraries(${NAME} ${BOOST_LIBRARIES})
set_property(TARGET ${NAME} PROPERTY CXX_STANDARD 20)