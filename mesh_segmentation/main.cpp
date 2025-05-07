#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/mesh_segmentation.h>
#include <CGAL/property_map.h>

// For visualization
#include <QGLViewer/qglviewer.h>
#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QColorDialog>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QMessageBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <cmath>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef CGAL::Surface_mesh<Point> Mesh;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;

// Property maps for segmentation
typedef Mesh::Property_map<face_descriptor, double> Face_double_map;
typedef Mesh::Property_map<face_descriptor, std::size_t> Face_index_map;

// For visualization
typedef CGAL::Exact_predicates_inexact_constructions_kernel::FT FT;
typedef CGAL::qglviewer::Vec Vec;
typedef CGAL::qglviewer::Quaternion Quaternion;

// Random color generation
std::vector<QColor> generate_random_colors(int n) {
    std::vector<QColor> colors;
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<> distrib(60, 255);
    
    for (int i = 0; i < n; ++i) {
        colors.push_back(QColor(distrib(gen), distrib(gen), distrib(gen)));
    }
    return colors;
}

// Mesh viewer widget
class MeshViewerWidget : public CGAL::QGLViewer {
public:
    MeshViewerWidget(QWidget* parent = nullptr) : CGAL::QGLViewer(parent), mesh(nullptr), show_segments(true) {}
    
    void setMesh(Mesh* mesh_ptr) {
        mesh = mesh_ptr;
        if (mesh) {
            // Compute bounding box
            CGAL::Bbox_3 bbox = CGAL::Polygon_mesh_processing::bbox(*mesh);
            
            // Set camera to view the entire mesh
            setSceneBoundingBox(
                qglviewer::Vec(bbox.xmin(), bbox.ymin(), bbox.zmin()),
                qglviewer::Vec(bbox.xmax(), bbox.ymax(), bbox.zmax())
            );
            camera()->showEntireScene();
            
            // Update display
            update();
        }
    }
    
    void setSegmentColors(const std::vector<QColor>& colors) {
        segment_colors = colors;
        update();
    }
    
    void setSegmentMap(Face_index_map* map) {
        segment_map = map;
        update();
    }
    
    void toggleSegments(bool show) {
        show_segments = show;
        update();
    }
    
protected:
    void init() override {
        // Set background color
        setBackgroundColor(QColor(240, 240, 240));
        
        // Set some default camera settings
        camera()->setType(qglviewer::Camera::PERSPECTIVE);
        camera()->setUpVector(qglviewer::Vec(0, 0, 1));
        camera()->setPosition(qglviewer::Vec(0, -2, 0));
        camera()->lookAt(qglviewer::Vec(0, 0, 0));
        
        // Light setup
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        GLfloat light_position[] = { 0.0f, 0.0f, 1.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        
        // Material setup
        GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat mat_shininess[] = { 50.0f };
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
        
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        
        // Enable smooth shading
        glShadeModel(GL_SMOOTH);
    }
    
    void draw() override {
        if (!mesh || !segment_map) return;
        
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        // Draw the mesh with segments colored
        glBegin(GL_TRIANGLES);
        for (face_descriptor fd : mesh->faces()) {
            // Get segment ID
            std::size_t segment_id = (*segment_map)[fd];
            
            // Set color based on segment
            if (show_segments && segment_id < segment_colors.size()) {
                QColor color = segment_colors[segment_id];
                glColor3f(color.redF(), color.greenF(), color.blueF());
            } else {
                glColor3f(0.8f, 0.8f, 0.8f); // Default gray
            }
            
            // Compute face normal
            Vector normal = CGAL::Polygon_mesh_processing::compute_face_normal(fd, *mesh);
            glNormal3d(normal.x(), normal.y(), normal.z());
            
            // Draw triangle
            halfedge_descriptor h = mesh->halfedge(fd);
            for (int i = 0; i < 3; ++i) {
                Point p = mesh->point(mesh->target(h));
                glVertex3d(p.x(), p.y(), p.z());
                h = mesh->next(h);
            }
        }
        glEnd();
        
        // Draw wireframe
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(1.0f);
        
        glBegin(GL_TRIANGLES);
        for (face_descriptor fd : mesh->faces()) {
            halfedge_descriptor h = mesh->halfedge(fd);
            for (int i = 0; i < 3; ++i) {
                Point p = mesh->point(mesh->target(h));
                glVertex3d(p.x(), p.y(), p.z());
                h = mesh->next(h);
            }
        }
        glEnd();
    }
    
    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_S) {
            show_segments = !show_segments;
            update();
        } else {
            CGAL::QGLViewer::keyPressEvent(e);
        }
    }
    
private:
    Mesh* mesh;
    Face_index_map* segment_map;
    std::vector<QColor> segment_colors;
    bool show_segments;
};

// Main application window
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent), mesh(nullptr) {
        // Initialize CGAL Qt resources
        CGAL::Qt::init_resources();
        
        // Create the central widget (mesh viewer)
        viewer = new MeshViewerWidget(this);
        setCentralWidget(viewer);
        
        // Create dock widget for controls
        QDockWidget* controlDock = new QDockWidget("Segmentation Controls", this);
        controlDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        
        QWidget* controlWidget = new QWidget(controlDock);
        QVBoxLayout* controlLayout = new QVBoxLayout(controlWidget);
        
        // SDF parameters group
        QGroupBox* sdfGroup = new QGroupBox("SDF Parameters", controlWidget);
        QFormLayout* sdfLayout = new QFormLayout(sdfGroup);
        
        // Number of rays
        QSpinBox* raysSpinBox = new QSpinBox(sdfGroup);
        raysSpinBox->setRange(10, 500);
        raysSpinBox->setValue(25);
        raysSpinBox->setSingleStep(5);
        sdfLayout->addRow("Number of rays:", raysSpinBox);
        
        // Cone angle
        QDoubleSpinBox* coneAngleSpinBox = new QDoubleSpinBox(sdfGroup);
        coneAngleSpinBox->setRange(0.1, 2.0);
        coneAngleSpinBox->setValue(0.7);
        coneAngleSpinBox->setSingleStep(0.1);
        sdfLayout->addRow("Cone angle:", coneAngleSpinBox);
        
        // Segmentation parameters group
        QGroupBox* segGroup = new QGroupBox("Segmentation Parameters", controlWidget);
        QFormLayout* segLayout = new QFormLayout(segGroup);
        
        // Number of clusters
        QSpinBox* clustersSpinBox = new QSpinBox(segGroup);
        clustersSpinBox->setRange(2, 100);
        clustersSpinBox->setValue(10);
        clustersSpinBox->setSingleStep(1);
        segLayout->addRow("Number of clusters:", clustersSpinBox);
        
        // Smoothing lambda
        QDoubleSpinBox* lambdaSpinBox = new QDoubleSpinBox(segGroup);
        lambdaSpinBox->setRange(0.0, 1.0);
        lambdaSpinBox->setValue(0.3);
        lambdaSpinBox->setSingleStep(0.05);
        segLayout->addRow("Smoothing lambda:", lambdaSpinBox);
        
        // Buttons
        QPushButton* loadButton = new QPushButton("Load STL", controlWidget);
        QPushButton* segmentButton = new QPushButton("Segment Mesh", controlWidget);
        QCheckBox* showSegmentsCheckBox = new QCheckBox("Show Segments", controlWidget);
        showSegmentsCheckBox->setChecked(true);
        
        // Add widgets to layout
        controlLayout->addWidget(loadButton);
        controlLayout->addWidget(sdfGroup);
        controlLayout->addWidget(segGroup);
        controlLayout->addWidget(segmentButton);
        controlLayout->addWidget(showSegmentsCheckBox);
        controlLayout->addStretch();
        
        // Set the control widget
        controlDock->setWidget(controlWidget);
        addDockWidget(Qt::RightDockWidgetArea, controlDock);
        
        // Status bar
        statusBar()->showMessage("Ready");
        
        // Connect signals and slots
        connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadSTL);
        connect(segmentButton, &QPushButton::clicked, [=]() {
            if (mesh) {
                segmentMesh(
                    raysSpinBox->value(),
                    coneAngleSpinBox->value(),
                    clustersSpinBox->value(),
                    lambdaSpinBox->value()
                );
            } else {
                QMessageBox::warning(this, "Error", "No mesh loaded");
            }
        });
        connect(showSegmentsCheckBox, &QCheckBox::toggled, viewer, &MeshViewerWidget::toggleSegments);
        
        // Set window properties
        setWindowTitle("CGAL Mesh Segmentation");
        resize(1024, 768);
    }
    
    ~MainWindow() {
        if (mesh) delete mesh;
    }
    
private slots:
    void loadSTL() {
        QString filename = QFileDialog::getOpenFileName(
            this, "Open STL File", "", "STL Files (*.stl);;All Files (*)");
        
        if (filename.isEmpty()) return;
        
        // Clean up previous mesh if any
        if (mesh) {
            delete mesh;
            mesh = nullptr;
        }
        
        // Create new mesh
        mesh = new Mesh();
        
        // Load the STL file
        if (!CGAL::Polygon_mesh_processing::IO::read_polygon_mesh(filename.toStdString(), *mesh)) {
            QMessageBox::critical(this, "Error", "Failed to load STL file");
            delete mesh;
            mesh = nullptr;
            return;
        }
        
        // Check if mesh is valid
        if (!CGAL::is_triangle_mesh(*mesh)) {
            QMessageBox::critical(this, "Error", "The mesh is not triangulated");
            delete mesh;
            mesh = nullptr;
            return;
        }
        
        // Update viewer
        viewer->setMesh(mesh);
        statusBar()->showMessage(QString("Loaded mesh with %1 vertices and %2 faces")
            .arg(mesh->number_of_vertices())
            .arg(mesh->number_of_faces()));
    }
    
    void segmentMesh(int num_rays, double cone_angle, int num_clusters, double lambda) {
        if (!mesh) return;
        
        statusBar()->showMessage("Computing SDF values...");
        QApplication::processEvents();
        
        // Create property maps for SDF values and segment IDs
        Face_double_map sdf_property_map;
        sdf_property_map = mesh->add_property_map<face_descriptor, double>("f:sdf").first;
        
        // Compute SDF values
        CGAL::sdf_values(*mesh, sdf_property_map, num_rays, cone_angle);
        
        statusBar()->showMessage("Segmenting mesh...");
        QApplication::processEvents();
        
        // Create a property map for segment IDs
        if (mesh->property_map<face_descriptor, std::size_t>("f:segment").second) {
            mesh->remove_property_map<face_descriptor, std::size_t>(
                mesh->property_map<face_descriptor, std::size_t>("f:segment").first);
        }
        Face_index_map segment_property_map = 
            mesh->add_property_map<face_descriptor, std::size_t>("f:segment").first;
        
        // Segment the mesh
        std::size_t num_segments = CGAL::segmentation_from_sdf_values(
            *mesh, sdf_property_map, segment_property_map, num_clusters, lambda);
        
        // Generate colors for segments
        std::vector<QColor> colors = generate_random_colors(num_segments);
        
        // Update viewer
        viewer->setSegmentMap(&segment_property_map);
        viewer->setSegmentColors(colors);
        viewer->update();
        
        statusBar()->showMessage(QString("Mesh segmented into %1 parts").arg(num_segments));
    }
    
private:
    MeshViewerWidget* viewer;
    Mesh* mesh;
};

// Main function
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
