#include "MeshLoader.h"
#include "MeshViewer.h"
#include "MeshTransformer.h"
#include "MeshSegmenter.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./mesh_app <operation> <input.off> [output.off]\n";
        std::cerr << "Operations:\n";
        std::cerr << "  view        - View and transform the mesh\n";
        std::cerr << "  segment     - Segment the mesh\n";
        return EXIT_FAILURE;
    }

    std::string operation = argv[1];
    std::string input_file = argv[2];
    std::string output_file = (argc >= 4) ? argv[3] : "output.off";

    Mesh mesh;
    if (!loadMesh(input_file, mesh)) {
        std::cerr << "Failed to load mesh from " << input_file << "\n";
        return EXIT_FAILURE;
    }

    if (operation == "view") {
        if (!viewAndTransformMesh(mesh)) {
            std::cerr << "Mesh viewing failed.\n";
            return EXIT_FAILURE;
        }
        if (!saveMesh(output_file, mesh)) {
            std::cerr << "Failed to save transformed mesh to " << output_file << "\n";
            return EXIT_FAILURE;
        }
    } else if (operation == "segment") {
        if (!segmentMesh(mesh)) {
            std::cerr << "Mesh segmentation failed.\n";
            return EXIT_FAILURE;
        }
        if (!saveMesh(output_file, mesh)) {
            std::cerr << "Failed to save segmented mesh to " << output_file << "\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << "Unknown operation: " << operation << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
