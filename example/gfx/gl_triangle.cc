#include <glad/glad.c>

#include <glfw/glfw3.h>
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
  if (!glfwInit()) {
    std::cerr << "Cound not start GLFW3" << std::endl;
    return 1;
  }
  // Only for mac I need this?
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
  if (!window) {
    std::cout << "Failed to open window with GLFW3" << std::endl;
    return 1;
  }
  glfwMakeContextCurrent(window);  // glad must be called after this.
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return 1;
  }
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
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

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(shader_program);
    // Bind the vertex array object.
    glBindVertexArray(vao);
    // glPolygonMode(GL_FRONT, GL_LINE);
    // Draw them points.
    // glDrawArrays(ENUM, starting_index, number_indices_to_draw)
    // glDrawArrays(GL_POINTS, 0, 3); // Can't see anything!
    // glDrawArrays(GL_LINES, 0, 3); // Only draws one line!
    // glDrawArrays(GL_LINE_STRIP, 0, 3); // Only draws two lines!
    // glDrawArrays(GL_LINE_LOOP, 0, 3); // Draws 3 lines making tri!
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Draws the triangle
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 3); // Draws the triangle
    // glDrawArrays(GL_TRIANGLE_FAN, 0, 3); // Draws the triangle
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}
