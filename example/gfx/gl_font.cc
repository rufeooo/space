#include <string>

#include "platform/window.cc"
#include "gl/utils.cc"
#include "gl/shader_cache.cc"
#include "tga_loader.cc"

#include <iostream>
#include <vector>
#include <string>

const char* vertex_shader =
  "#version 410\n"
  "layout (location = 0) in vec4 text_pos;"
  "out vec2 texture_coordinates;"
  "void main() {"
  "	texture_coordinates = text_pos.zw;"
  "	gl_Position = vec4(text_pos.xy, 0.0 , 1.0);"
  "}";

const char* fragment_shader =
  "#version 410\n"
  "in vec2 texture_coordinates;"
  "uniform sampler2D basic_texture;"
  "out vec4 frag_colour;"
  "void main() {"
  "	vec4 texel = texture(basic_texture, texture_coordinates);"
  "	frag_colour = texel;"
  "}";

struct TextPoint {
  TextPoint() = default;
  TextPoint(GLfloat x, GLfloat y, GLfloat s, GLfloat t) :
    x(x), y(y), s(s), t(t) {};
  GLfloat x;
  GLfloat y;
  GLfloat s;
  GLfloat t;

  void Pr() { printf("%.1f,%.1f,%.1f,%.1f\n", x, y, s, t); }
};

void
DrawString(const char* msg, const FntMetadata& fm, float start_x, float start_y,
           float texture_width, float texture_height)
{
  int msg_len = strlen(msg);

  for (int i = 0; i < msg_len; ++i) {
    TextPoint text_point[6];

    const FntMetadataRow* row = &fm.rows[msg[i]];

    float tex_x = row->x / texture_width;
    float tex_y = row->y / texture_height;
    float tex_w = row->width / texture_width;
    float tex_h = row->height / texture_height;

    float offset_start_x = start_x - row->xoffset / texture_width;
    float offset_start_y = start_y - row->yoffset / texture_height;

    text_point[0] = TextPoint(offset_start_x, offset_start_y,
                              tex_x, tex_y);
    text_point[1] = TextPoint(offset_start_x + tex_w, offset_start_y,
                              tex_x + tex_w, tex_y);
    text_point[2] = TextPoint(offset_start_x + tex_w, offset_start_y - tex_h,
                              tex_x + tex_w, tex_y + tex_h);
    text_point[3] = TextPoint(offset_start_x + tex_w, offset_start_y - tex_h,
                              tex_x + tex_w, tex_y + tex_h);
    text_point[4] = TextPoint(offset_start_x, offset_start_y - tex_h,
                              tex_x, tex_y + tex_h);
    text_point[5] = TextPoint(offset_start_x, offset_start_y,
                              tex_x, tex_y);

    glBufferData(GL_ARRAY_BUFFER, sizeof(text_point), text_point, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw the character 
    
    start_x += tex_w;
  }
}

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
  uint16_t texture_width, texture_height;
  LoadTGA("example/gfx/characters_0.tga", &image_data, &texture_width, &texture_height);
  FntMetadata fm = LoadFntMetadata("example/gfx/characters.fnt");
  GLuint tex = 0;
  glGenTextures (1, &tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D (
    GL_TEXTURE_2D,
    0,
    GL_RED,
    texture_width,
    texture_height,
    0,
    GL_RED,
    GL_UNSIGNED_BYTE,
    image_data
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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


  
  
  GLuint text_vbo = 0;
  glGenBuffers(1, &text_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, text_vbo);

  GLuint text_vao;
  glGenVertexArrays(1, &text_vao);
  glBindVertexArray(text_vao);

  glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(0);

  float start_x = 0.f;
  float start_y = 0.f;

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(p);
    glUniform1i(tex_loc, 0); // use active texture 0

    DrawString("Hello, World!", fm, start_x, start_y, texture_width, texture_height);

    window::SwapBuffers();
  }
  return 0;
}
