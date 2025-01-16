# Minimum required cmake version
cmake_minimum_required(VERSION 3.10)
project(main_node)

# Find required packages
find_package(cppzmq REQUIRED)

set(Eigen_Folder /home/enes/Desktop/PhD/Repos/eigen) # Adjust this path as necessary


# Include directories for Eigen and Orbbec SDK
include_directories(
    ${Eigen_Folder}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )


# Define the OBPointCloud executable and link with libraries
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link libraries to OBPointCloud
target_link_libraries(${PROJECT_NAME}
    ${PCL_LIBRARIES}
    cppzmq
)

# Install configuration
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
