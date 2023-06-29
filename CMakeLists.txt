

if(WIN32)
    message("win 32 build!")
    set(CMAKE_MAKE_PROGRAM "C:/MinGW/bin/mingw32-make.exe")
    set(CMAKE_CXX_COMPILER "C:/MinGW/bin/g++.exe")
    set(CMAKE_C_COMPILER "C:/MinGW/bin/gcc.exe")
elseif(UNIX)
    message("linux build!")
endif()

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(NAME congestion_calculator)
project(${NAME})

#set(CMAKE_SYSTEM_NAME Linux)
#set(CMAKE_SYSTEM_PROCESSOR arm)

#option(USE_CUDNN "Use cuDNN" ON)

#set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda-11.8)
#set(CMAKE_PREFIX_PATH "/usr/local/cuda-11.8")
#set(CUDNN_LIBRARY "/usr/lib/x86_64-linux-gnu/libcudnn.so")
#set(CMAKE_CUDA_COMPILER /usr/local/cuda-11.8/bin/nvcc)
set(Torch_DIR /home/yul/libtorch/share/cmake/Torch)
#set(OpenCV_DIR /home/yul/opencv_arm/lib/cmake/opencv4)
set(Boost_DIR /home/yul/test/boost/build/lib/cmake/Boost-1.83.0)
#set (BOOST_ROOT /home/yul/boost)
#set (Boost_LIBRARY_DIR /home/yul/build/lib)
#set (Boost_INCLUDE_DIR /home/yul/build/include)

#set(SQLITE_ROOT /home/yul/sqlite)
set(CMAKE_GENERATOR Ninja)

#find_package(Threads REQUIRED)
find_package(Torch 2.0.0 REQUIRED)
#find_package(CUDA REQUIRED)
#find_package(CUDNN REQUIRED)
find_package(OpenCV 4.5.1 REQUIRED)
find_package(Boost 1.82.0 REQUIRED COMPONENTS json locale)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS})

include_directories(include)
#include_directories(${SQLITE_ROOT})
#include_directories(${CUDA_INCLUDE_DIRS})
#include_directories(${CUDNN_INCLUDE_DIRS})
#include_directories(${Boost_INCLUDE_DIR})

#link_directories(${Boost_LIBRARY_DIR})
#link_directories(/home/yul/build/lib)
#add_library(sqlite3 ${SQLITE_ROOT}/sqlite3.c)
add_executable(${NAME} main.cpp VideoProcessor.cpp ImageProcessor.cpp ClientHandler.cpp)

#target_link_libraries(${NAME} ${CUDA_LIBRARIES} ${CUDNN_LIBRARIES})
#target_link_libraries(${NAME} Threads::Threads)
target_link_libraries(${NAME} ${TORCH_LIBRARIES})
target_link_libraries(${NAME} ${OpenCV_LIBS})
#target_link_libraries(${NAME} sqlite3)
target_link_libraries(${NAME} ${Boost_LIBRARIES})
set_property(TARGET ${NAME} PROPERTY CXX_STANDARD 20)