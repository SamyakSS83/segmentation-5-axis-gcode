#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/mesh_segmentation.h>
#include <CGAL/draw_surface_mesh.h>
#include <CGAL/IO/Color.h>
#include <CGAL/property_map.h>
#include "stl_reader.h"    // https://github.com/sreiter/stl_reader
#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Mesh;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " input.stl\n";
        return EXIT_FAILURE;
    }
    const char* filename = argv[1];

    // --- Step 1: Read STL file ---
    stl_reader::StlMesh<float, unsigned> stl_mesh;
    if (!stl_mesh.read_file(filename)) {
        std::cerr << "Error: cannot read STL file.\n";
        return EXIT_FAILURE;
    }

    // --- Step 2: Convert to CGAL Surface_mesh ---
    Mesh mesh;
    std::vector<Mesh::Vertex_index> vhandle(stl_mesh.num_vrts());
    for (size_t i = 0; i < stl_mesh.num_vrts(); ++i) {
        const float* coords = stl_mesh.raw_coords() + 3 * i;
        vhandle[i] = mesh.add_vertex(Point(coords[0], coords[1], coords[2]));
    }
    for (size_t i = 0; i < stl_mesh.num_tris(); ++i) {
        const unsigned* tri = stl_mesh.tri_corner_inds(i);
        mesh.add_face(vhandle[tri[0]], vhandle[tri[1]], vhandle[tri[2]]);
    }

    if (mesh.number_of_faces() == 0) {
        std::cerr << "Error: No faces in mesh.\n";
        return EXIT_FAILURE;
    }

    // --- Step 3: Compute and normalize SDF values ---
    auto sdf_map = mesh.add_property_map<Mesh::Face_index, double>("f:sdf", 0.0).first;
    std::pair<double, double> mm = CGAL::sdf_values(mesh, sdf_map);
    double min_sdf = mm.first, max_sdf = mm.second;
    for (auto f : mesh.faces()) {
        double v = get(sdf_map, f);
        put(sdf_map, f, (v - min_sdf) / (max_sdf - min_sdf));
    }

    std::vector<double> data;
    data.reserve(mesh.number_of_faces());
    for (auto f : mesh.faces()) data.push_back(get(sdf_map, f));

    // --- Step 4: Auto-select cluster count using silhouette ---
    int best_k = 2;
    double best_sil = -1.0;
    int N = (int)data.size();
    int K_max = std::min(N, 10);
    std::vector<int> labels(N, 0);

    for (int k = 2; k <= K_max; ++k) {
        std::vector<double> centers(k);
        for (int c = 0; c < k; ++c) centers[c] = (2 * c + 1) / double(2 * k);

        for (int iter = 0; iter < 50; ++iter) {
            bool changed = false;
            for (int i = 0; i < N; ++i) {
                double mind = std::abs(data[i] - centers[0]);
                int bestc = 0;
                for (int c = 1; c < k; ++c) {
                    double d = std::abs(data[i] - centers[c]);
                    if (d < mind) { mind = d; bestc = c; }
                }
                if (labels[i] != bestc) { labels[i] = bestc; changed = true; }
            }
            if (!changed) break;

            std::vector<double> sum(k, 0.0);
            std::vector<int> cnt(k, 0);
            for (int i = 0; i < N; ++i) {
                sum[labels[i]] += data[i];
                cnt[labels[i]]++;
            }
            for (int c = 0; c < k; ++c) {
                if (cnt[c] > 0) centers[c] = sum[c] / cnt[c];
            }
        }

        // Silhouette score
        double sil_sum = 0.0;
        int valid = 0;
        for (int i = 0; i < N; ++i) {
            int c = labels[i];
            double a = 0.0; int ac = 0;
            for (int j = 0; j < N; ++j) if (j != i && labels[j] == c) { a += std::abs(data[i] - data[j]); ac++; }
            if (ac == 0) continue;
            a /= ac;

            double b = std::numeric_limits<double>::infinity();
            for (int c2 = 0; c2 < k; ++c2) {
                if (c2 == c) continue;
                double sumd = 0.0; int cnt2 = 0;
                for (int j = 0; j < N; ++j) if (labels[j] == c2) { sumd += std::abs(data[i] - data[j]); cnt2++; }
                if (cnt2 > 0) b = std::min(b, sumd / cnt2);
            }
            double sil = (b - a) / std::max(a, b);
            sil_sum += sil;
            valid++;
        }

        if (valid > 0) {
            double sil_avg = sil_sum / valid;
            if (sil_avg > best_sil) {
                best_sil = sil_avg;
                best_k = k;
            }
        }
    }

    if (best_sil < 0) {
        std::cerr << "Silhouette computation failed. Falling back to 2 clusters.\n";
        best_k = 2;
    }
    std::cout << "Chosen number of clusters: " << best_k << " (silhouette=" << best_sil << ")\n";

    // --- Step 5: Segment mesh ---
    auto segment_map = mesh.add_property_map<Mesh::Face_index, std::size_t>("f:segment", 0).first;
    std::size_t num_segments = 0;
    double smoothing = 0.3;

    try {
        num_segments = CGAL::segmentation_from_sdf_values(mesh, sdf_map, segment_map, best_k, smoothing);
    } catch (...) {
        std::cerr << "CGAL segmentation failed. Falling back to simple thresholding.\n";
        double threshold = 0.5;
        for (auto f : mesh.faces()) {
            double v = get(sdf_map, f);
            put(segment_map, f, v < threshold ? 0 : 1);
        }
        num_segments = 2;
    }

    std::cout << "Number of segments found: " << num_segments << "\n";

    // --- Step 6: Color and visualize ---
    std::vector<CGAL::IO::Color> colors = {
        CGAL::IO::red(), CGAL::IO::green(), CGAL::IO::blue(),
        CGAL::IO::yellow(), CGAL::IO::orange(), CGAL::IO::gray()
    };
    auto fcolormap = mesh.add_property_map<Mesh::Face_index, CGAL::IO::Color>("f:color", CGAL::IO::white()).first;
    for (auto f : mesh.faces()) {
        std::size_t id = get(segment_map, f);
        put(fcolormap, f, colors[id % colors.size()]);
    }

    std::cout << "Launching viewer...\n";
    CGAL::draw(mesh);

    return EXIT_SUCCESS;
}
