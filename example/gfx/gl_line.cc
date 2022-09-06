#include "common/macro.h"
#include "platform/window.cc"

#include <iostream>

const char* vertex_shader =
    "#version 410\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";

const char* fragment_shader =
    "#version 410\n"
    "out vec4 frag_colour;"
    "void main() {"
    " frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

int
main()
{
  int window_result = window::Create("Hello Line", 640, 480, false);
  std::cout << "Window create: " << window_result << std::endl;
  // Only for mac I need this?
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  GLfloat points[] = {
      0.0f, 0.5f, 0.0f, 0.0f, -0.5f, 0.0f,
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
  // Create and compile vertex shader.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  // Create and compile fragment shader.
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  // Link them together.
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  int running = 1;
  glClearColor(0.0, 0.0, 0.0, 1.0);
  while (running) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(shader_program);
    // Bind the vertex array object.
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2);  // Draws the triangle
    window::SwapBuffers();
  }
  return 0;
}
