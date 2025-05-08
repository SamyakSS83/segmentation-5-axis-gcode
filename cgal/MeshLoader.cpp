#include "MeshLoader.h"
#include <CGAL/IO/Polyhedron_iostream.h>  // If using Polyhedron
#include <CGAL/IO/Surface_mesh_io.h>      // ✅ For Surface_mesh
#include <fstream>

bool loadMesh(const std::string& filename, Mesh& mesh) {
    std::ifstream input(filename);
    if (!input || !(input >> mesh) || mesh.is_empty()) {
        return false;
    }
    return true;
}

bool saveMesh(const std::string& filename, const Mesh& mesh) {
    std::ofstream output(filename);
    if (!output || !(output << mesh)) {
        return false;
    }
    return true;
}
