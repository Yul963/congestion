set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(NAME congestion_calculator)
project(${NAME})
set(CMAKE_GENERATOR Ninja)
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH \${ORIGIN})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    set(Torch_DIR "../libtorch-debug/share/cmake/Torch")
else()
    set(Torch_DIR "../libtorch/share/cmake/Torch")
endif()

#option(USE_CUDNN "Use cuDNN" ON)
#set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda-11.8)
#set(CMAKE_PREFIX_PATH "/usr/local/cuda-11.8")
#set(CUDNN_LIBRARY "/usr/lib/x86_64-linux-gnu/libcudnn.so")
#set(CMAKE_CUDA_COMPILER /usr/local/cuda-11.8/bin/nvcc)

set(Boost_DIR "../boost/lib/cmake/Boost-1.82.0")
set(OpenCV_DIR "../opencv/build")

#find_package(CUDA REQUIRED)
#find_package(CUDNN REQUIRED)
find_package(Torch 2.0.1 REQUIRED)
find_package(OpenCV 4.8.0 REQUIRED)
find_package(Boost 1.82.0 REQUIRED COMPONENTS json locale)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS})

include_directories(include)
#include_directories(${CUDA_INCLUDE_DIRS})
#include_directories(${CUDNN_INCLUDE_DIRS})

add_executable(${NAME} main.cpp VideoProcessor.cpp ImageProcessor.cpp ClientHandler.cpp)

#target_link_libraries(${NAME} ${CUDA_LIBRARIES} ${CUDNN_LIBRARIES})
target_link_libraries(${NAME} ${TORCH_LIBRARIES})
message(${TORCH_LIBRARIES})
target_link_libraries(${NAME} ${OpenCV_LIBS})
message(${OpenCV_LIBS})
target_link_libraries(${NAME} ${Boost_LIBRARIES})
message(${Boost_LIBRARIES})

if(WIN32)
    #file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    #add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS} $<TARGET_FILE_DIR:${NAME}>)
    #set(OpenCV_DLLS ${OpenCV_DIR}/x64/vc16/bin/*.dll)
    #add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OpenCV_DLLS} $<TARGET_FILE_DIR:${NAME}>)
    add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OpenCV_DIR}/x64/vc16/bin/opencv_videoio_ffmpeg480_64.dll $<TARGET_FILE_DIR:${NAME}>)  
elseif(UNIX)
    
endif()