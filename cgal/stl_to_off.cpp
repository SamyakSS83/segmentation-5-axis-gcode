#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/STL.h>      // for read_STL
#include <CGAL/IO/OFF.h>      // for write_OFF
#include <vector>
#include <array>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: stl_to_off <input.stl> <output.off>\n";
        return 1;
    }
    const char* stl_file = argv[1];
    const char* off_file = argv[2];

    typedef CGAL::Simple_cartesian<double> Kernel;
    typedef Kernel::Point_3 Point;
    CGAL::Surface_mesh<Point> mesh;

    // Read STL into a list of points and triangles
    std::vector<Point> points;
    std::vector<std::array<int,3>> triangles;
    if (!CGAL::IO::read_STL(stl_file, points, triangles)) {
        std::cerr << "Error: cannot read STL file '" << stl_file << "'\n";
        return 1;
    }

    // Build the surface mesh from the triangle soup
    std::vector<CGAL::Surface_mesh<Point>::Vertex_index> vmap;
    vmap.reserve(points.size());
    for (const auto& p : points) {
        // Add each point as a vertex in the mesh
        vmap.push_back(mesh.add_vertex(p));
    }
    for (const auto& tri : triangles) {
        // Add each triangle as a face (indices assume 0-based)
        mesh.add_face(vmap[tri[0]], vmap[tri[1]], vmap[tri[2]]);
    }

    // Write the mesh to OFF
    std::ofstream out(off_file);
    if (!out || !CGAL::IO::write_OFF(out, mesh)) {
        std::cerr << "Error: cannot write OFF file '" << off_file << "'\n";
        return 1;
    }
    return 0;
}
