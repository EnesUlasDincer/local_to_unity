# Minimum required cmake version
cmake_minimum_required(VERSION 3.10)
project(main_node)

# Find required packages
find_package(cppzmq REQUIRED)

find_package(CUDA REQUIRED)
find_package(CUDAToolkit REQUIRED)

#export CC=/usr/bin/gcc-11
#export CXX=/usr/bin/g++-11

set(Eigen_Folder /home/enes/Desktop/PhD/Repos/eigen) # Adjust this path as necessary
# CUDA configuration
set(CUDA_NVCC_FLAGS "-arch=compute_75 --expt-relaxed-constexpr" CACHE STRING "nvcc flags" FORCE)
set(CUDA_VERBOSE_BUILD ON CACHE BOOL "nvcc verbose" FORCE)
set(LIB_TYPE STATIC)

cmake_policy(SET CMP0146 NEW)
cmake_policy(SET CMP0074 NEW)

# Set OrbbecSDK paths
set(OrbbecSDK_LIBS_DIR /home/enes/Desktop/ORBBEC/OrbbecSDK/lib/linux_x64)
set(OrbbecSDK_INCLUDE /home/enes/Desktop/ORBBEC/OrbbecSDK/include) # Adjust this path as necessary
set(OrbbecSDK_INCLUDE_HPP /home/enes/Desktop/ORBBEC/OrbbecSDK/include/libobsensor/hpp) # Adjust this path as necessary


# Include directories for Eigen and Orbbec SDK
include_directories(
    ${Eigen_Folder}
    ${OrbbecSDK_INCLUDE}
    ${OrbbecSDK_INCLUDE_HPP}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

# Build CUDA library for voxel grid
cuda_add_library(VoxelGrid ${LIB_TYPE} src/voxel_grid.cu)

# Define the OBPointCloud executable and link with libraries
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link libraries to OBPointCloud
target_link_libraries(${PROJECT_NAME}
    ${PCL_LIBRARIES}
    ${OrbbecSDK_LIBS_DIR}/libOrbbecSDK.so
    ${OrbbecSDK_LIBS_DIR}/libdepthengine.so
    ${OrbbecSDK_LIBS_DIR}/libob_usb.so
    ${OrbbecSDK_LIBS_DIR}/liblive555.so
    cppzmq
    VoxelGrid
)

# Install configuration
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
