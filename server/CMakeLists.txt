cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(mars LANGUAGES CXX CUDA)

set(Torch_DIR /home/patrik/libtorch_cpp/libtorch/share/cmake/Torch)
find_package(Torch REQUIRED)
find_package(X11 REQUIRED) # for the CImg
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_LINK_WHAT_YOU_USE TRUE)

#find_package(cuda REQUIRED)

#CMAKE_CXX_FLAGS:STRING=-pthread
#if(CMAKE_COMPILER_IS_GNUCC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
#endif(CMAKE_COMPILER_IS_GNUCC)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "main.cpp" "stream/*.cpp" "control/*.cpp" "stream/*.cu")
add_executable(mars ${SOURCES})
target_link_libraries(mars "pthread")
target_link_libraries(mars "jpeg")        # needed for the CImg
target_link_libraries(mars "stb")         # sudo apt install libstb-dev
#target_link_libraries(mars "OpenImageIO") # sudo apt install libopenimageio-dev # to draw text on images
target_link_libraries(mars "${TORCH_LIBRARIES}")
target_link_libraries(mars "${X11_LIBRARIES}") # needed for the CImg
set_property(TARGET mars PROPERTY CXX_STANDARD 17)
