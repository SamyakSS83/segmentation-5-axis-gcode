cmake_minimum_required(VERSION 3.10)
project(MeshSegmentationTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CGAL and Eigen
find_package(CGAL REQUIRED COMPONENTS Core)
find_package(Eigen3 REQUIRED)

# Add executable with all source files
add_executable(MeshSegmentationTool
    main.cpp
    stl_to_off.cpp
    MeshLoader.cpp
    MeshSegmenter.cpp
    MeshTransformer.cpp
    MeshViewer.cpp
)

# Include Eigen if found
target_include_directories(MeshSegmentationTool PRIVATE ${EIGEN3_INCLUDE_DIR})

# Link with CGAL
target_link_libraries(MeshSegmentationTool CGAL::CGAL CGAL::CGAL_Core)
