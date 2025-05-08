#ifndef MESHTRANSFORMER_H
#define MESHTRANSFORMER_H

#include "MeshLoader.h"
#include <Eigen/Dense>

void applyTransformation(Mesh& mesh, const Eigen::Matrix4d& transform);

#endif // MESHTRANSFORMER_H
