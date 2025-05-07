cmake_minimum_required(VERSION 3.1...3.23)
project(mesh_segmentation)

# Find CGAL and its components
find_package(CGAL REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL Xml)
find_package(QGLViewer REQUIRED)

# Create the executable
add_executable(mesh_segmentation main.cpp)

# Link with CGAL libraries
target_link_libraries(mesh_segmentation PRIVATE
  CGAL::CGAL 
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Xml
  QGLViewer::QGLViewer
)

# Set C++14 standard
target_compile_features(mesh_segmentation PRIVATE cxx_std_14)
