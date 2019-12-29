#include "ui.h"

#include "gl/shader.h"
#include "tga_loader.cc"
#include "shader.h"

namespace ui {

struct Font {
  uint16_t texture_width;
  uint16_t texture_height;
  FntMetadata metadata;
  GLuint texture;
  GLenum texture_slot;
  GLuint program;
  GLuint texture_uniform;
  GLuint vbo;
  GLuint vao;
};

struct UI {
  // Font.
  Font font;
};

static UI kUI;

bool
Initialize()
{
  Font& font = kUI.font;
  uint8_t* image_data;
  uint16_t texture_width, texture_height;
  LoadTGA("example/gfx/characters_0.tga", &image_data, &font.texture_width,
          &font.texture_height);
  font.metadata = LoadFntMetadata("example/gfx/characters.fnt");
  glGenTextures (1, &font.texture);
  font.texture_slot = GL_TEXTURE0;
  glActiveTexture(font.texture_slot);
  glBindTexture(GL_TEXTURE_2D, font.texture);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RED,
    font.texture_width,
    font.texture_height,
    0,
    GL_RED,
    GL_UNSIGNED_BYTE,
    image_data
  );
  free(image_data); // GL has the texture data now. Free it from process.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &rgg::kFontVertexShader,
                         &vert_shader)) {
    return false;
  }
  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &rgg::kFontFragmentShader,
                         &frag_shader)) {
    return false;
  }
  if (!gl::LinkShaders({vert_shader, frag_shader}, &font.program)) {
    return false;
  }
  font.texture_uniform = glGetUniformLocation(font.program, "basic_texture");

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  // Setup font vbo / vao.
  glGenBuffers(1, &font.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, font.vbo);

  glGenVertexArrays(1, &font.vao);
  glBindVertexArray(font.vao);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  return true;
}

void
DrawString(const char* msg, float x, float y, const math::Vec4f& color)
{
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

  auto& font = kUI.font;
  glBindTexture(GL_TEXTURE_2D, font.texture);
  glBindVertexArray(font.vao);

  int msg_len = strlen(msg);
  for (int i = 0; i < msg_len; ++i) {
    TextPoint text_point[6];

    const FntMetadataRow* row = &font.metadata.rows[msg[i]];

    float tex_x = (float)row->x / font.texture_width;
    float tex_y = (float)row->y / font.texture_height;
    float tex_w = (float)row->width / font.texture_width;
    float tex_h = (float)row->height / font.texture_height;

    float offset_start_x = x - (float)row->xoffset / font.texture_width;
    float offset_start_y = y - (float)row->yoffset / font.texture_height;

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

#if 0
    text_point[0].Pr();
    text_point[1].Pr();
    text_point[2].Pr();
    text_point[3].Pr();
    text_point[4].Pr();
    text_point[5].Pr();
#endif

    glBufferData(GL_ARRAY_BUFFER, sizeof(text_point), text_point, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw the character 
    x += tex_w;
  }
}

void
Text(const char* msg, float x, float y, const math::Vec4f& color)
{
  auto& font = kUI.font;
  glUseProgram(font.program);
  DrawString(msg, x, y, color);
}

}
