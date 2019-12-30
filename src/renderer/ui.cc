#include "ui.h"

#include "gl/shader.h"
#include "math/mat_ops.h"
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
  GLuint matrix_uniform;
  GLuint vbo;
  GLuint vao;
};

struct UI {
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
  if (!gl::LinkShaders(&font.program, 2, vert_shader, frag_shader)) {
    return false;
  }
  font.texture_uniform = glGetUniformLocation(font.program, "basic_texture");
  font.matrix_uniform = glGetUniformLocation(font.program, "matrix");

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
    TextPoint(GLfloat x, GLfloat y, GLfloat u, GLfloat v) :
      x(x), y(y), u(u), v(v) {};
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
    void Pr() { printf("%.1f,%.1f,%.1f,%.1f\n", x, y, u, v); }
  };

  auto& font = kUI.font;
  glBindTexture(GL_TEXTURE_2D, font.texture);
  glBindVertexArray(font.vao);

  auto sz = window::GetWindowSize();
  math::Mat4f projection = math::CreateOrthographicMatrix<float>(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  glUniformMatrix4fv(font.matrix_uniform, 1, GL_FALSE, &projection[0]);

#if 0
  math::Print4x4Matrix(kUI.projection);
  printf("\n");
#endif

  int msg_len = strlen(msg);
  for (int i = 0; i < msg_len; ++i) {
    TextPoint text_point[6];

    const FntMetadataRow* row = &font.metadata.rows[msg[i]];

    // Scale to get uv coordinatoes.
    float tex_x = (float)row->x / font.texture_width;
    float tex_y = (float)row->y / font.texture_height;
    float tex_w = (float)row->width / font.texture_width;
    float tex_h = (float)row->height / font.texture_height;

    // Verts are subject to projection given by an orhthographic matrix
    // using the screen width and height.
    float v_w = (float)row->width;
    float v_h = (float)row->height;

    //float xoffset = (float)row->xoffset / (2.f * font.texture_width);
    float offset_start_x = x;
    float offset_start_y = y;

#if 0
    printf("id=%i char=%c width=%i height=%i xoffset=%i yoffset=%i"
           " start_x=%.3f start_y=%.3f tex_w=%.3f tex_h=%.3f\n",
           msg[i], msg[i], row->width, row->height, row->xoffset,
           row->yoffset, offset_start_x, offset_start_y, tex_w, tex_h);
#endif

    text_point[0] = TextPoint(offset_start_x, offset_start_y,
                              tex_x, tex_y);
    text_point[1] = TextPoint(offset_start_x + v_w, offset_start_y,
                              tex_x + tex_w, tex_y);
    text_point[2] = TextPoint(offset_start_x + v_w, offset_start_y - v_h,
                              tex_x + tex_w, tex_y + tex_h);
    text_point[3] = TextPoint(offset_start_x + v_w, offset_start_y - v_h,
                              tex_x + tex_w, tex_y + tex_h);
    text_point[4] = TextPoint(offset_start_x, offset_start_y - v_h,
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
    x += v_w;
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
