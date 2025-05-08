cmake_minimum_required(VERSION 3.12)
project(MeshSegmentationExample)

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
if(CGAL_Qt5_FOUND)
  add_executable(segmentation_demo main.cpp)
  target_link_libraries(segmentation_demo PUBLIC 
        CGAL::CGAL            # CGAL core
        CGAL::CGAL_Basic_viewer)  # Qt-based viewer:contentReference[oaicite:4]{index=4}
endif()
