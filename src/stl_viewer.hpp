#ifndef STL_VIEWER_HPP
#define STL_VIEWER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "stl_reader.h"

/**
 * @brief STL Viewer namespace containing all viewer functionality
 */
namespace stl_viewer {

/**
 * @brief Compiles a GLSL shader
 * @param type Shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @param source GLSL source code
 * @return Shader ID
 */
GLuint compileShader(GLenum type, const char* source);

/**
 * @brief Creates a shader program from vertex and fragment shader sources
 * @param vertexSource Vertex shader GLSL source
 * @param fragmentSource Fragment shader GLSL source
 * @return Program ID
 */
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

/**
 * @brief Processes keyboard input for camera control and application state
 * @param window GLFW window handle
 */
void processInput(GLFWwindow* window);

/**
 * @brief Handles window resize events
 * @param window GLFW window handle
 * @param width New width
 * @param height New height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

/**
 * @brief Loads an STL model into vertex and normal arrays
 * @param mesh STL mesh data
 * @param vertices Output container for vertex data
 * @param normals Output container for normal data
 */
void loadModel(const stl_reader::StlMesh<float, unsigned int>& mesh,
               std::vector<float>& vertices,
               std::vector<float>& normals);

/**
 * @brief Toggles between fullscreen and windowed mode
 * @param window GLFW window handle
 */
void toggleFullscreen(GLFWwindow* window);

/**
 * @brief Initializes OpenGL rendering
 * @param window GLFW window handle
 * @param vertices Vertex data
 * @param normals Normal data
 * @return VAO ID
 */
GLuint initializeRendering(GLFWwindow* window, 
                           const std::vector<float>& vertices,
                           const std::vector<float>& normals);

/**
 * @brief Centers the camera on the model
 * @param vertices Vertex data
 * @return Center position and size of the model
 */
struct ModelStats {
    float centerX, centerY, centerZ;
    float size;
};
ModelStats centerCamera(const std::vector<float>& vertices);

} // namespace stl_viewer

#endif // STL_VIEWER_HPP