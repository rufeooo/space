#pragma once

namespace rgg {

inline constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  uniform vec4 color;
  out vec4 color_out;
  void main() {
    color_out = color;
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

inline constexpr const char* kFragmentShader = R"(
  #version 410
  in vec4 color_out;
	out vec4 frag_color;
  void main() {
   frag_color = color_out;
  }
)";

inline constexpr const char* kFontVertexShader = R"(
  #version 410
  layout (location = 0) in vec4 text_pos;
  out vec2 texture_coordinates;
  void main() {
  	texture_coordinates = text_pos.zw;
  	gl_Position = vec4(text_pos.xy, 0.0 , 1.0);
  }
)";

inline constexpr const char* kFontFragmentShader = R"(
  #version 410
  in vec2 texture_coordinates;
  uniform sampler2D basic_texture;
  out vec4 frag_color;
  void main() {
  	vec4 texel = texture(basic_texture, texture_coordinates);
  	frag_color = vec4(texel.x, texel.x, texel.x, 1.0);
  }
)";

}
