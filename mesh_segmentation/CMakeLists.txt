cmake_minimum_required(VERSION 3.1...3.23)
project(mesh_segmentation)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable Qt's automoc feature
set(CMAKE_AUTOMOC ON)

# Find CGAL and its components
find_package(CGAL REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL Xml)
find_package(QGLViewer REQUIRED)
find_package(OpenGL REQUIRED)  # Add this line to find OpenGL

# Create the executable
add_executable(mesh_segmentation main.cpp)

# Link with CGAL libraries
target_link_libraries(mesh_segmentation PRIVATE
  CGAL::CGAL 
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Xml
  QGLViewer::QGLViewer
  OpenGL::GL  # Add this line to link against OpenGL
)

# Set C++14 standard
target_compile_features(mesh_segmentation PRIVATE cxx_std_14)