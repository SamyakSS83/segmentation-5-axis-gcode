#include "MeshViewer.h"
#include <CGAL/draw_surface_mesh.h>

bool viewAndTransformMesh(Mesh& mesh) {
    try {
        CGAL::draw(mesh);
    } catch (...) {
        return false;
    }
    return true;
}
