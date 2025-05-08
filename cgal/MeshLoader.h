#ifndef MESHLOADER_H
#define MESHLOADER_H

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <string>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Mesh;

bool loadMesh(const std::string& filename, Mesh& mesh);
bool saveMesh(const std::string& filename, const Mesh& mesh);

#endif // MESHLOADER_H
