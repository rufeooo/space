#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <plog/Log.h>
#include <iostream>

void
GLFWErrorCallback(int error, const char* description)
{
  PLOGD << "GLFW ERROR: code " << error << " msg: " << description;
}

void
GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  PLOGD << "width: " << width << " height: " << height;
}

void
LogGLParams()
{
  GLenum params[] = {
      GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
      GL_MAX_CUBE_MAP_TEXTURE_SIZE,
      GL_MAX_DRAW_BUFFERS,
      GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
      GL_MAX_TEXTURE_IMAGE_UNITS,
      GL_MAX_TEXTURE_SIZE,
      GL_MAX_VARYING_FLOATS,
      GL_MAX_VERTEX_ATTRIBS,
      GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
      GL_MAX_VERTEX_UNIFORM_COMPONENTS,
      GL_MAX_VIEWPORT_DIMS,
      GL_STEREO,
  };
  const char* names[] = {
      "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
      "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
      "GL_MAX_DRAW_BUFFERS",
      "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
      "GL_MAX_TEXTURE_IMAGE_UNITS",
      "GL_MAX_TEXTURE_SIZE",
      "GL_MAX_VARYING_FLOATS",
      "GL_MAX_VERTEX_ATTRIBS",
      "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
      "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
      "GL_MAX_VIEWPORT_DIMS",
      "GL_STEREO",
  };
  PLOGD << glfwGetVersionString();
  for (int i = 0; i < 10; ++i) {
    int v = 0;
    glGetIntegerv(params[i], &v);
    PLOGD << names[i] << " " << v;
  }
  int v[2];
  v[0] = v[1] = 0;
  glGetIntegerv(params[10], v);
  PLOGD << names[10] << " " << v[0] << " " << v[1];
  unsigned char s = 0;
  glGetBooleanv(params[11], &s);
  PLOGD << names[11] << " " << (unsigned int)s;
}

int
main()
{
  plog::init(plog::debug, "logs/gl_log.log");
  glfwSetErrorCallback(GLFWErrorCallback);
  if (!glfwInit()) {
    std::cerr << "ERROR: could not start GLFW" << std::endl;
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  GLFWmonitor* mon = glfwGetPrimaryMonitor();
  const GLFWvidmode* vmode = glfwGetVideoMode(mon);
  GLFWwindow* window = glfwCreateWindow(vmode->width, vmode->height,
                                        "Extended GL init", mon, nullptr);
  return 0;
}
