#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include "stl_reader.h"

// OpenGL and GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stl_viewer.hpp"

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

namespace stl_viewer {

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;
float cameraSpeed = 0.05f;

// Window state tracking
bool fullscreen = false;
int windowedWidth = WIDTH;
int windowedHeight = HEIGHT;
int windowedPosX = 100;
int windowedPosY = 100;

// Load model data from the STL mesh
void loadModel(const stl_reader::StlMesh<float, unsigned int>& mesh,
               std::vector<float>& vertices,
               std::vector<float>& normals) {
    size_t numTriangles = mesh.num_tris();
    
    vertices.reserve(numTriangles * 9);  // 3 vertices per triangle, 3 coords per vertex
    normals.reserve(numTriangles * 9);   // 3 normals per triangle, 3 coords per normal
    
    for (size_t itri = 0; itri < numTriangles; ++itri) {
        const float* n = mesh.tri_normal(itri);
        
        for (int i = 0; i < 3; ++i) {
            const float* coords = mesh.tri_corner_coords(itri, i);
            
            // Add vertex coordinates
            vertices.push_back(coords[0]);
            vertices.push_back(coords[1]);
            vertices.push_back(coords[2]);
            
            // Add normal (same for all vertices in the triangle for flat shading)
            normals.push_back(n[0]);
            normals.push_back(n[1]);
            normals.push_back(n[2]);
        }
    }
}

// Shader compilation function
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check for compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    return shader;
}

// Create shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check for linking errors
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

// Process keyboard input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    float currentSpeed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 2.0f;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraUp * currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraUp * currentSpeed;
        
    // Axis-aligned views (90-degree rotations)
    static bool key1Released = true;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && key1Released) {
        // Front view (looking along -Z axis)
        yaw = -90.0f; pitch = 0.0f;
        cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        key1Released = false;
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1Released = true;
    }
    
    // Add F key for fullscreen toggle
    static bool fKeyReleased = true;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && fKeyReleased) {
        toggleFullscreen(window);
        fKeyReleased = false;
    } else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyReleased = true;
    }
    
    // Continue with other keys...
    // (Keep your existing key 2-6 implementation here)
}

// Function to toggle fullscreen
void toggleFullscreen(GLFWwindow* window) {
    if (!fullscreen) {
        // Save windowed mode dimensions and position
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
        glfwGetWindowPos(window, &windowedPosX, &windowedPosY);

        // Get primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        // Switch to fullscreen
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        fullscreen = true;
    } else {
        // Switch back to windowed mode
        glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
        fullscreen = false;
    }
}

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Implementation for ModelStats centerCamera function
ModelStats centerCamera(const std::vector<float>& vertices) {
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::min();
    
    for (size_t i = 0; i < vertices.size(); i += 3) {
        minX = std::min(minX, vertices[i]);
        maxX = std::max(maxX, vertices[i]);
        minY = std::min(minY, vertices[i+1]);
        maxY = std::max(maxY, vertices[i+1]);
        minZ = std::min(minZ, vertices[i+2]);
        maxZ = std::max(maxZ, vertices[i+2]);
    }
    
    ModelStats stats;
    stats.centerX = (minX + maxX) / 2.0f;
    stats.centerY = (minY + maxY) / 2.0f;
    stats.centerZ = (minZ + maxZ) / 2.0f;
    stats.size = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    
    return stats;
}

} // end namespace stl_viewer

// OpenGL shader source (keep outside namespace)
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    out vec3 Normal;
    out vec3 FragPos;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 Normal;
    in vec3 FragPos;
    
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    
    void main() {
        // Ambient
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;
        
        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <model.stl>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    std::vector<float> vertices;
    std::vector<float> normals;

    try {
        stl_reader::StlMesh<float, unsigned int> mesh(filename);

        size_t numTriangles = mesh.num_tris();
        std::cout << "Loaded STL: " << filename << "\n";
        std::cout << "Triangles: " << numTriangles << "\n";

        // Load model data into vertex and normal arrays
        stl_viewer::loadModel(mesh, vertices, normals);

        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return -1;
        }

        // Set OpenGL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a window
        GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "STL Viewer", NULL, NULL);
        if (window == NULL) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        
        // Set callbacks
        glfwSetFramebufferSizeCallback(window, stl_viewer::framebuffer_size_callback);

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return -1;
        }

        // Create and compile shaders
        GLuint shaderProgram = stl_viewer::createShaderProgram(vertexShaderSource, fragmentShaderSource);
        
        // Create VAO, VBO, normalVBO
        GLuint VAO, VBO, normalVBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &normalVBO);
        
        glBindVertexArray(VAO);
        
        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        
        // Center the camera on the model
        stl_viewer::ModelStats modelStats = stl_viewer::centerCamera(vertices);
        stl_viewer::cameraPos = glm::vec3(modelStats.centerX, modelStats.centerY, modelStats.centerZ + modelStats.size * 2.0f);
        
        // Main render loop
        while (!glfwWindowShouldClose(window)) {
            // Process input
            stl_viewer::processInput(window);
            
            // Render
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Use shader
            glUseProgram(shaderProgram);
            
            // Set uniforms
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-modelStats.centerX, -modelStats.centerY, -modelStats.centerZ)); // Center the model
            
            glm::mat4 view = glm::lookAt(stl_viewer::cameraPos, 
                                         stl_viewer::cameraPos + stl_viewer::cameraFront, 
                                         stl_viewer::cameraUp);
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                                   (float)WIDTH / (float)HEIGHT, 
                                                   0.1f, 
                                                   modelStats.size * 10.0f);
            
            // Pass matrices to shader
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            
            // Set lighting uniforms
            glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 
                        stl_viewer::cameraPos.x, 
                        stl_viewer::cameraPos.y, 
                        stl_viewer::cameraPos.z);
            glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 
                        stl_viewer::cameraPos.x, 
                        stl_viewer::cameraPos.y, 
                        stl_viewer::cameraPos.z);
            glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
            glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.5f, 1.0f);
            
            // Draw model
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
            
            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        // Clean up
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteProgram(shaderProgram);
        
        glfwTerminate();
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to read STL file: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}