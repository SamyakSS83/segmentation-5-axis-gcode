// MeshSegmenter.cpp
#include "MeshSegmenter.h"

#include <CGAL/Surface_mesh_segmentation.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/Color.h>
#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

bool segmentMesh(Mesh& mesh) {
    if (!CGAL::is_triangle_mesh(mesh)) {
        std::cerr << "Input geometry is not triangulated.\n";
        return false;
    }

    auto sdf_map = mesh.add_property_map<Mesh::Face_index, double>("f:sdf", 0.0).first;
    auto range = CGAL::sdf_values(mesh, sdf_map);
    double min_sdf = range.first, max_sdf = range.second;

    // Normalize SDF values to [0, 1]
    for (auto f : mesh.faces()) {
        double norm_val = (sdf_map[f] - min_sdf) / (max_sdf - min_sdf);
        sdf_map[f] = norm_val;
    }

    // Extract normalized SDF values
    std::vector<double> sdf_values;
    for (auto f : mesh.faces()) sdf_values.push_back(sdf_map[f]);

    // Try k from 2 to 10 and use silhouette score
    int best_k = 2;
    double best_sil = -1.0;
    std::vector<int> labels(sdf_values.size(), 0);
    int N = (int)sdf_values.size();

    for (int k = 2; k <= std::min(10, N); ++k) {
        std::vector<double> centers(k);
        for (int i = 0; i < k; ++i) centers[i] = (2 * i + 1) / (2.0 * k);

        std::vector<int> curr_labels(N);
        for (int iter = 0; iter < 50; ++iter) {
            bool changed = false;
            for (int i = 0; i < N; ++i) {
                double dmin = std::abs(sdf_values[i] - centers[0]);
                int bestc = 0;
                for (int j = 1; j < k; ++j) {
                    double d = std::abs(sdf_values[i] - centers[j]);
                    if (d < dmin) {
                        dmin = d;
                        bestc = j;
                    }
                }
                if (curr_labels[i] != bestc) {
                    curr_labels[i] = bestc;
                    changed = true;
                }
            }
            if (!changed) break;
            std::vector<double> sum(k, 0.0);
            std::vector<int> count(k, 0);
            for (int i = 0; i < N; ++i) {
                sum[curr_labels[i]] += sdf_values[i];
                count[curr_labels[i]]++;
            }
            for (int j = 0; j < k; ++j) {
                if (count[j]) centers[j] = sum[j] / count[j];
            }
        }

        // Compute silhouette
        double sil_sum = 0.0;
        int valid = 0;
        for (int i = 0; i < N; ++i) {
            double a = 0.0; int ac = 0;
            for (int j = 0; j < N; ++j) {
                if (j != i && curr_labels[i] == curr_labels[j]) {
                    a += std::abs(sdf_values[i] - sdf_values[j]); ac++;
                }
            }
            if (ac > 0) a /= ac; else continue;

            double b = std::numeric_limits<double>::infinity();
            for (int c = 0; c < k; ++c) {
                if (c == curr_labels[i]) continue;
                double bsum = 0.0; int bc = 0;
                for (int j = 0; j < N; ++j) {
                    if (curr_labels[j] == c) {
                        bsum += std::abs(sdf_values[i] - sdf_values[j]); bc++;
                    }
                }
                if (bc > 0) b = std::min(b, bsum / bc);
            }

            double sil = (b - a) / std::max(a, b);
            sil_sum += sil; valid++;
        }

        if (valid > 0) {
            double avg_sil = sil_sum / valid;
            if (avg_sil > best_sil) {
                best_sil = avg_sil;
                best_k = k;
                labels = curr_labels;
            }
        }
    }

    std::cout << "Optimal number of segments: " << best_k << " (silhouette=" << best_sil << ")\n";

    auto segment_map = mesh.add_property_map<Mesh::Face_index, std::size_t>("f:segment", 0).first;
    double smoothing_lambda = 0.3;
    std::size_t n_segments = CGAL::segmentation_from_sdf_values(mesh, sdf_map, segment_map, best_k, smoothing_lambda);

    std::cout << "Segments generated: " << n_segments << "\n";

    auto fcolormap = mesh.add_property_map<Mesh::Face_index, CGAL::IO::Color>("f:color", CGAL::IO::white()).first;
    std::vector<CGAL::IO::Color> palette = {
        CGAL::IO::red(), CGAL::IO::green(), CGAL::IO::blue(),
        CGAL::IO::yellow(), CGAL::IO::magenta(), CGAL::IO::cyan(), CGAL::IO::gray()
    };

    for (auto f : mesh.faces()) {
        fcolormap[f] = palette[segment_map[f] % palette.size()];
    }

    return true;
}
