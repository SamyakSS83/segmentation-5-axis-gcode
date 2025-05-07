# Try to find QGLViewer library
# Once done this will define:
#  QGLVIEWER_FOUND - if system found QGLViewer library
#  QGLVIEWER_INCLUDE_DIRS - The QGLViewer include directories
#  QGLVIEWER_LIBRARIES - The libraries needed to use QGLViewer

find_path(QGLVIEWER_INCLUDE_DIR
  NAMES QGLViewer/qglviewer.h
  PATHS
  /usr/include
  /usr/local/include
  /opt/local/include
  /sw/include
  ENV QGLVIEWERROOT
)

find_library(QGLVIEWER_LIBRARY
  NAMES QGLViewer qglviewer-qt5 QGLViewer-qt5
  PATHS
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  /sw/lib
  ENV QGLVIEWERROOT
  ENV LD_LIBRARY_PATH
  ENV LIBRARY_PATH
  PATH_SUFFIXES QGLViewer QGLViewer/release
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QGLViewer DEFAULT_MSG
  QGLVIEWER_LIBRARY QGLVIEWER_INCLUDE_DIR)

if(QGLVIEWER_FOUND)
  set(QGLVIEWER_LIBRARIES ${QGLVIEWER_LIBRARY})
  set(QGLVIEWER_INCLUDE_DIRS ${QGLVIEWER_INCLUDE_DIR})
  
  if(NOT TARGET QGLViewer::QGLViewer)
    add_library(QGLViewer::QGLViewer UNKNOWN IMPORTED)
    set_target_properties(QGLViewer::QGLViewer PROPERTIES
      IMPORTED_LOCATION "${QGLVIEWER_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${QGLVIEWER_INCLUDE_DIR}")
  endif()
endif()

mark_as_advanced(QGLVIEWER_INCLUDE_DIR QGLVIEWER_LIBRARY)