cmake_minimum_required(VERSION 3.15)
project(MeshSegmenter)

find_package(CGAL REQUIRED COMPONENTS Qt5)  # Explicit Qt5 component [1][3]
include(${CGAL_USE_FILE})

add_executable(mesh_segmenter main.cpp)

# Essential viewer definition [1][4]
add_definitions(-DCGAL_USE_BASIC_VIEWER)

target_compile_features(mesh_segmenter PRIVATE cxx_std_17)
target_link_libraries(mesh_segmenter
  PRIVATE 
  CGAL::CGAL
  CGAL::CGAL_Qt5  # Explicit Qt5 linking [1][3]
  CGAL::CGAL_Basic_viewer
)
