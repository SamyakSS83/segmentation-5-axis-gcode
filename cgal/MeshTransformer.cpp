#include "MeshTransformer.h"

void applyTransformation(Mesh& mesh, const Eigen::Matrix4d& transform) {
    for (auto v : mesh.vertices()) {
        auto p = mesh.point(v);
        Eigen::Vector4d point(p.x(), p.y(), p.z(), 1.0);
        Eigen::Vector4d transformed = transform * point;
        mesh.point(v) = Point(transformed[0], transformed[1], transformed[2]);
    }
}
