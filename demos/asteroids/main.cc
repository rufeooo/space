#include <iostream>

#include "components/common/transform_component.h"
#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "game/gl_game.h"
#include "math/mat_ops.h"
#include "math/vec.h"
#include "renderer/gl_shader_cache.h"
#include "renderer/gl_utils.h"

static float kRotationSpeed = 1.5f;
static float kShipAcceleration = 0.00004f;
static float kSecsToMaxSpeed = 3.f;

constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  void main() {
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

constexpr const char* kVertexShaderName = "vert";

constexpr const char* kFragmentShader = R"(
  #version 410
	out vec4 frag_color;
  void main() {
   frag_color = vec4(1.0, 1.0, 1.0, 1.0);
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

struct PhysicsComponent {
  PhysicsComponent() = default;
  float acceleration = 0.f;
  float velocity = 0.f;
  float acceleration_speed = kShipAcceleration;
  float max_velocity = kShipAcceleration * kSecsToMaxSpeed * 60.f;
};

class Asteroids : public game::GLGame {
 public:
  Asteroids() : game::GLGame(1280, 720) {};
  bool Initialize() override {
    if (!GLGame::Initialize()) return false;
    ecs::Assign<component::ViewComponent>(
      camera_,
      math::Vec3f(0.0f, 0.0f, 10.0f),
      math::Quatf(0.0f, math::Vec3f(0.0f, 0.0f, -1.0f)));
    ecs::Assign<InputComponent>(player_);
    ecs::Assign<ShooterComponent>(player_);
    ecs::Assign<PhysicsComponent>(player_);
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
        {kVertexShaderName, kFragmentShaderName})) {
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
    matrix_location_ = glGetUniformLocation(
        program_reference, "matrix");
    // Print program info
    std::cout << shader_cache_.GetProgramInfo(kProgramName)
              << std::endl;
    uint32_t vao_reference = renderer::CreateGeometryVAO({
        0.0f, 0.08f, 0.0f,
        0.03f, -0.03f, 0.0f,
        0.00f, -0.005f, 0.0f,
        -0.03f, -0.03f, 0.0f,
    });
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
    ecs::Enumerate<InputComponent,
                   PhysicsComponent,
                   component::TransformComponent>(
        [this](ecs::Entity ent,
           InputComponent& input,
           PhysicsComponent& physics,
           component::TransformComponent& transform) {
      // Apply rotation. 
      int state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_A);
      if (state == GLFW_PRESS) {
        transform.orientation.Rotate(-kRotationSpeed);
      }
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_D);
      if (state == GLFW_PRESS) {
        transform.orientation.Rotate(kRotationSpeed);
      }

      /*std::cout
          << "Up: "
          << transform.orientation.Up().String() << " "
          << "Forward: "
          << transform.orientation.Forward().String() << " "
          << "Left: "
          << transform.orientation.Left().String()
          << std::endl;*/

      // Set acceleration to facing direction times acceleration_speed.
      auto u = transform.orientation.Up();
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_W);
      if (state == GLFW_PRESS) {
        physics.acceleration = physics.acceleration_speed;
      }
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_S);
      if (state == GLFW_PRESS) {
        physics.acceleration = -physics.acceleration_speed;
      }
    });
    return true;
  }

  // Game logic
  bool Update() override {
    ecs::Enumerate<PhysicsComponent,
                   component::TransformComponent>(
        [this](ecs::Entity ent,
           PhysicsComponent& physics,
           component::TransformComponent& transform) {
      if (physics.velocity < physics.max_velocity ||
          (physics.acceleration < 0.f && physics.velocity > 0.f)) {
        physics.velocity += physics.acceleration;
      }
      transform.position += transform.orientation.Up()
                            * physics.velocity;
    });

    return true;
  }

  // Render logic
  bool Render() override {
    if (!GLGame::Render()) return false;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ecs::Enumerate<component::TriangleComponent,
                   component::TransformComponent>(
        [&](ecs::Entity ent,
            component::TriangleComponent& comp,
            component::TransformComponent& transform) {
      glUseProgram(comp.program_reference);
      auto* view_component = ecs::Get<component::ViewComponent>(
          camera_);
      math::Mat4f view = math::CreateViewMatrix(
          view_component->position, view_component->orientation);
      math::Mat4f model =
          math::CreateTranslationMatrix(transform.position) *
          math::CreateRotationMatrix(transform.orientation);
      math::Mat4f matrix = model * view;
      glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &matrix[0]);
      glBindVertexArray(comp.vao_reference);
      glDrawArrays(GL_LINE_LOOP, 0, 4);
    });
    glfw_renderer_.SwapBuffers();
    auto* view_component = ecs::Get<component::ViewComponent>(camera_);
    math::Vec3f camera_forward = view_component->orientation.Forward();
    math::Vec3f camera_up = view_component->orientation.Up();
    math::Vec3f camera_left = view_component->orientation.Left();
    char title[256];
    sprintf(
        title,
        "Asteroids MS Per Frame: %lld FPS: %.2f Forward: %s Up: %s "
        "Left: %s",
            ms_per_frame().count(),
            fps(),
            camera_forward.String().c_str(),
            camera_up.String().c_str(),
            camera_left.String().c_str());
    glfwSetWindowTitle(glfw_renderer_.window(), title);
    return true;
  }

 private:
  int matrix_location_;
  ecs::Entity camera_ = 0;
  ecs::Entity player_ = 1;
  renderer::GLShaderCache shader_cache_;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();
  return 0;
}
