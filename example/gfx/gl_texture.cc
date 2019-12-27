#include <string>

#include "platform/window.cc"
#include "gl/utils.cc"
#include "gl/shader_cache.cc"
#include "tga_loader.cc"

#include <iostream>
#include <vector>

const char* vertex_shader =
  "#version 410\n"
  "layout (location = 0) in vec3 vertex_position;"
  "layout (location = 1) in vec2 vt;"
  "out vec2 texture_coordinates;"
  "void main() {"
  "	texture_coordinates = vt;"
  "	gl_Position =vec4 (vertex_position, 1.0);"
  "}";

const char* fragment_shader =
  "#version 410\n"
  "in vec2 texture_coordinates;"
  "uniform sampler2D basic_texture;"
  "out vec4 frag_colour;"
  "void main() {"
  "	vec4 texel = texture (basic_texture, texture_coordinates);"
  "	frag_colour = texel;"
  "}";

int
main(int argc, char** argv)
{
  int window_result = window::Create("Textures", 640, 480);
  std::cout << "Window create: " << window_result << std::endl;
  // Only for mac I need this?
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  uint8_t* image_data;
  uint16_t x, y;
  LoadTGA("example/gfx/characters_0.tga", &image_data, &x, &y);
  GLuint tex = 0;
  glGenTextures (1, &tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D (
    GL_TEXTURE_2D,
    0,
    GL_RED,
    x,
    y,
    0,
    GL_RED,
    GL_UNSIGNED_BYTE,
    image_data
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  GLfloat points[] = { -1.f, 1.f, 0.0f, 1.f, 1.f, 0.0f, 1.f, -1.f, 1.f,
                        1.f, -1.f, 0.0f, -1.f, -1.f, 0.0f, -1.f, 1.f, 0.0f };

  GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                          1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };
  // Create VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), points, GL_STATIC_DRAW);

  GLuint texcoords_vbo;
	glGenBuffers( 1, &texcoords_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
	glBufferData( GL_ARRAY_BUFFER, 12 * sizeof( GLfloat ), texcoords,
								GL_STATIC_DRAW );

  GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray( vao );
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL ); // normalise!
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

  gl::ShaderCache cache;

  std::string a = "a";
  std::string b = "b";
  cache.CompileShader(a, gl::ShaderType::VERTEX, vertex_shader);
  cache.CompileShader(b, gl::ShaderType::FRAGMENT, fragment_shader);
  cache.LinkProgram("prog", std::vector({a, b}));
  uint32_t p;
  cache.GetProgramReference("prog", &p);
  std::cout << cache.GetProgramInfo("prog") << std::endl;

  int tex_loc = glGetUniformLocation (p, "basic_texture");

  glClearColor(0.0, 0.0, 0.0, 1.0);
  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(p);
    glUniform1i (tex_loc, 0); // use active texture 0
    // Bind the vertex array object.
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // Draws the texture 
    window::SwapBuffers();
  }
  return 0;
}
