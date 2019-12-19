#include <iostream>

#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"
#include "renderer/gl_shader_cache.h"
#include "renderer/glfw_renderer.h"

constexpr const char* kVertexShader =
    "#version 410\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";

constexpr const char* kFragmentShader =
    "#version 410\n"
    "out vec4 frag_colour;"
    "void main() {"
    " frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

int
main()
{
  renderer::GLFWRenderer glfw_renderer;
  if (!glfw_renderer.Start(480, 680, "Test")) {
    std::cout << "unable to start renderer" << std::endl;
    return 1;
  }
  GLfloat points[] = {
      0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
  };
  // Create VBO.
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);
  // Create VAO bind vbo to it.
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  renderer::GLShaderCache gl_shader_cache;
  if (!gl_shader_cache.CompileShader("vert", renderer::ShaderType::VERTEX,
                                     kVertexShader) ||
      !gl_shader_cache.CompileShader("frag", renderer::ShaderType::FRAGMENT,
                                     kFragmentShader)) {
    std::cout << "unable to compile shaders." << std::endl;
    return 1;
  }

  if (!gl_shader_cache.LinkShaders("program", {"frag", "vert"})) {
    std::cout << "unable to link program." << std::endl;
    return 1;
  }

  while (!glfw_renderer.HasStopped()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    if (!gl_shader_cache.UseProgram("program")) {
      std::cout << "unable to use program for rendering." << std::endl;
      return 1;
    }
    // Bind the vertex array object.
    glBindVertexArray(vao);
    // Draw them points.
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwPollEvents();
    glfw_renderer.SwapBuffers();
  }
  glfwTerminate();
  return 0;
}
