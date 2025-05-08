#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/OFF.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
#include <CGAL/mesh_segmentation.h>  // Correct segmentation header [2]
#include <CGAL/draw_surface_mesh.h>  // Required for viewer

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Surface_mesh;
typedef boost::graph_traits<Surface_mesh>::face_descriptor face_descriptor;

namespace PMP = CGAL::Polygon_mesh_processing;

void view_mesh(const Surface_mesh& mesh) {
    CGAL::draw(mesh);  // Requires CGAL_USE_BASIC_VIEWER definition [1]
}

void transform_mesh(Surface_mesh& mesh, const CGAL::Aff_transformation_3<Kernel>& trans) {
    PMP::transform(trans, mesh);
}

void segment_mesh(Surface_mesh& mesh, int num_clusters = 5) {
    // Property map for SDF values
    auto sdf_pmap = mesh.add_property_map<face_descriptor, double>("f:sdf").first;
    CGAL::sdf_values(mesh, sdf_pmap);
    
    // Property map for segment IDs
    auto segment_pmap = mesh.add_property_map<face_descriptor, std::size_t>("f:segment_id").first;
    CGAL::segmentation_from_sdf_values(mesh, sdf_pmap, segment_pmap, num_clusters);
}

int main(int argc, char* argv[]) {
    Surface_mesh mesh;
    const std::string input = (argc > 1) ? argv[1] : "input.off";
    
    if(!CGAL::IO::read_OFF(input, mesh)) {
        std::cerr << "Failed to read OFF file" << std::endl;
        return EXIT_FAILURE;
    }

    // Command-line processing
    bool view_flag = false;
    bool transform_flag = false;
    bool segment_flag = false;
    int clusters = 5;
    Kernel::Vector_3 translation(0, 0, 0);

    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg == "--view") view_flag = true;
        if(arg == "--translate" && i+3 < argc) {
            translation = Kernel::Vector_3(
                std::stod(argv[++i]),
                std::stod(argv[++i]),
                std::stod(argv[++i])
            );
            transform_flag = true;
        }
        if(arg == "--segment") {
            segment_flag = true;
            if(i+1 < argc) clusters = std::stoi(argv[++i]);
        }
    }

    if(transform_flag) {
        PMP::transform(CGAL::Aff_transformation_3<Kernel>(CGAL::TRANSLATION, translation), mesh);
        CGAL::IO::write_OFF("transformed.off", mesh);
    }

    if(segment_flag) {
        segment_mesh(mesh, clusters);
    }

    if(view_flag) {
        view_mesh(mesh);  // Now works with basic viewer
    }

    return EXIT_SUCCESS;
}
