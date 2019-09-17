#include <iostream>

#include "components/common/transform_component.h"
#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"
#include "ecs/ecs.h"
#include "game/gl_game.h"
#include "math/vec.h"
#include "renderer/gl_shader_cache.h"

constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  // uniform mat4 matrix;
  void main() {
    gl_Position = /* matrix * */vec4(vertex_position, 1.0);
  }
)";

constexpr const char* kVertexShaderName = "vert";

constexpr const char* kFragmentShader = R"(
  #version 410
	out vec4 frag_color;
  void main() {
   frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  }
)";

constexpr const char* kFragmentShaderName = "frag";

constexpr const char* kProgramName = "prog";

struct InputComponent {
  InputComponent() = default;
};

struct ShooterComponent {
  ShooterComponent() = default;
};

// Should this go into a common library?
uint32_t CreateTriangleVAO() {
  GLfloat points[] = {
    0.0f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
  };
  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points,
               GL_STATIC_DRAW);
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return vao;
}

class Asteroids : public game::GLGame {
 public:
  Asteroids() : game::GLGame(640, 480) {};
  bool Initialize() override {
    if (!GLGame::Initialize()) return false;
    ecs::Assign<InputComponent>(player_);
    ecs::Assign<ShooterComponent>(player_);
    ecs::Assign<component::TriangleComponent>(player_);
    ecs::Assign<component::TransformComponent>(player_);
    if (!shader_cache_.CompileShader(
        kVertexShaderName,
        renderer::ShaderType::VERTEX,
        kVertexShader)) {
      std::cout << "Unable to compile "
                << kVertexShaderName << std::endl;
      return false;
    }
    if (!shader_cache_.CompileShader(
        kFragmentShaderName,
        renderer::ShaderType::FRAGMENT,
        kFragmentShader)) {
      std::cout << "Unable to compile "
                << kFragmentShaderName << std::endl;
      return false;
    }
    if (!shader_cache_.LinkProgram(
        kProgramName,
        {kVertexShaderName, kFragmentShaderName},
        false)) {
      std::cout << "Unable to link: "
                << kProgramName
                << " info: "
                << shader_cache_.GetProgramInfo(kProgramName)
                << std::endl;
      return false;
    }
    uint32_t program_reference;
    if (!shader_cache_.GetProgramReference(
        kProgramName, &program_reference)) {
      return false;
    }
    // Print program info
    std::cout << shader_cache_.GetProgramInfo(kProgramName)
              << std::endl;
    uint32_t vao_reference = CreateTriangleVAO();
    ecs::Enumerate<component::TriangleComponent>(
        [vao_reference, program_reference]
            (ecs::Entity ent, component::TriangleComponent& comp) {
      comp.vao_reference = vao_reference;
      comp.program_reference = program_reference;
    });
    return true;
  }

  // Input logic
  bool ProcessInput() override {
    glfwPollEvents();
    return true;
  }

  // Game logic
  bool Update() override {
    return true;
  }

  // Render logic
  bool Render() override {
    if (!GLGame::Render()) return false;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ecs::Enumerate<component::TriangleComponent>(
        [](ecs::Entity ent, component::TriangleComponent& comp) {
      glUseProgram(comp.program_reference);
      glBindVertexArray(comp.vao_reference);
      glDrawArrays(GL_LINE_LOOP, 0, 3);
    });
    glfw_renderer_.SwapBuffers();
    return true;
  }

 private:
  ecs::Entity player_ = 1;
  renderer::GLShaderCache shader_cache_;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();
  return 0;
}
