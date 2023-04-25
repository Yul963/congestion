cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(NAME congestion_calculator)
project(${NAME})

set(Torch_DIR /home/yul/libtorch/share/cmake/Torch)
set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(CMAKE_GENERATOR "Ninja")

find_package(Torch REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

include_directories(${CUDA_INCLUDE_DIRS} include)
add_executable(${NAME} main.cpp vid_processing.cpp congestion_cal.cpp)
target_link_libraries(${NAME} "${TORCH_LIBRARIES}")
target_link_libraries(${NAME} ${OpenCV_LIBS})
set_property(TARGET ${NAME} PROPERTY CXX_STANDARD 14)

# The following code block is suggested to be used on Windows.
# According to https://github.com/pytorch/pytorch/issues/25457,
# the DLLs need to be copied to avoid memory errors.
if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(
        TARGET
        ${NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TORCH_DLLS}
        $<TARGET_FILE_DIR:${NAME}>
        )
endif (MSVC)