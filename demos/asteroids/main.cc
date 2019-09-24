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

static float kRotationSpeedDegrees = 1.5f;
static float kShipAcceleration = 0.00004f;
static float kSecsToMaxSpeed = 3.f;
static float kDampenVelocity = 0.00001f;

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
  math::Vec3f acceleration;
  math::Vec3f velocity;
  float acceleration_speed = kShipAcceleration;
  float max_velocity = kShipAcceleration * kSecsToMaxSpeed * 60.f;
};

struct AsteroidComponent {
  uint32_t vao_reference;
  uint32_t program_reference;
};


class Asteroids : public game::GLGame {
 public:
  Asteroids() : game::GLGame(800, 800) {};
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
    ecs::Assign<AsteroidComponent>(asteroid_);
    ecs::Assign<component::TransformComponent>(asteroid_);
    auto* physics = ecs::Assign<PhysicsComponent>(asteroid_);
    physics->velocity = math::Vec3f(
        -kShipAcceleration * 50.f, 0.f, 0.0f);
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
    uint32_t vao_ship_reference = renderer::CreateGeometryVAO({
        0.0f, 0.08f, 0.0f,
        0.03f, -0.03f, 0.0f,
        0.00f, -0.005f, 0.0f,
        -0.03f, -0.03f, 0.0f,
    });
    auto* triangle_component =
        ecs::Get<component::TriangleComponent>(player_);
    triangle_component->vao_reference = vao_ship_reference;
    triangle_component->program_reference = program_reference;
    uint32_t vao_asteroid_reference = renderer::CreateGeometryVAO({
        0.0f, 0.1f, 0.0f,
        0.07f, 0.08f, 0.0f,
        0.06f, -0.01f, 0.0f,
        0.11f, -0.005f, 0.0f,
        0.1f, -0.06f, 0.0f,
        0.05f, -0.08f, 0.0f,
        0.01f, -0.1f, 0.0f,
        -0.07f, -0.08f, 0.0f,
        -0.1f, -0.01f, 0.0f,
        -0.08f, 0.06f, 0.0f
    });
    auto* asteroid_component = ecs::Get<AsteroidComponent>(asteroid_);
    asteroid_component->vao_reference = vao_asteroid_reference;
    asteroid_component->program_reference = program_reference;
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
        transform.orientation.Rotate(-kRotationSpeedDegrees);
      }
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_D);
      if (state == GLFW_PRESS) {
        transform.orientation.Rotate(kRotationSpeedDegrees);
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
      physics.acceleration = math::Vec3f(0.f, 0.f, 0.f);
      if (state == GLFW_PRESS) {
        physics.acceleration = u * physics.acceleration_speed;
      }
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_S);
      if (state == GLFW_PRESS) {
        physics.acceleration = u * -physics.acceleration_speed;
      }
    });
    return true;
  }

  // Game logic
  bool Update() override {
    // Provide ship control to the entity with Input (the player.)
    ecs::Enumerate<PhysicsComponent, InputComponent>(
        [this](ecs::Entity ent, PhysicsComponent& physics,
               InputComponent& /*input*/) {
      // If the ship is not at max velocity and the ship has
      // acceleration.
      auto velocity_squared = math::LengthSquared(physics.velocity);
      if (velocity_squared < 
          physics.max_velocity * physics.max_velocity &&
          math::LengthSquared(physics.acceleration) > 0.f) {
        physics.velocity += physics.acceleration;
      } else if (math::LengthSquared(physics.velocity) > 0.f) {
        // Dampen velocity.
        auto vdir = physics.velocity;
        vdir.Normalize();
        physics.velocity -= vdir * kDampenVelocity;
        if (velocity_squared < kShipAcceleration * kShipAcceleration) {
          physics.velocity = math::Vec3f(0.f, 0.f, 0.f);
        }
      }
    });

    ecs::Enumerate<PhysicsComponent, component::TransformComponent>(
        [this](ecs::Entity ent, PhysicsComponent& physics,
               component::TransformComponent& transform) {
      transform.position += physics.velocity;
      //std::cout << transform.position.String() << std::endl;
      if (transform.position.x() <= -1.0f) { 
        transform.position.x() = 0.99f;
      } else if (transform.position.x() >= 1.0f) {
        transform.position.x() = -0.99f;
      }
      if (transform.position.y() <= -1.0f) { 
        transform.position.y() = 0.99f;
      } else if (transform.position.y() >= 1.0f) {
        transform.position.y() = -0.99f;
      }
    });


    return true;
  }

  // Render logic
  bool Render() override {
    if (!GLGame::Render()) return false;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto* view_component = ecs::Get<component::ViewComponent>(
          camera_);
    math::Mat4f view = math::CreateViewMatrix(
        view_component->position, view_component->orientation);
    math::Mat4f projection =
          math::CreatePerspectiveMatrix<float>(
              window_width_, window_height_);
    ecs::Enumerate<component::TriangleComponent,
                   component::TransformComponent>(
        [&](ecs::Entity ent,
            component::TriangleComponent& comp,
            component::TransformComponent& transform) {
      glUseProgram(comp.program_reference);
      math::Mat4f model =
          math::CreateTranslationMatrix(transform.position) *
          math::CreateRotationMatrix(transform.orientation);
      math::Mat4f matrix = model * view * projection;
      glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &matrix[0]);
      glBindVertexArray(comp.vao_reference);
      glDrawArrays(GL_LINE_LOOP, 0, 4);
    });
    ecs::Enumerate<AsteroidComponent,
                   component::TransformComponent>(
        [&](ecs::Entity ent,
            AsteroidComponent& comp,
            component::TransformComponent& transform) {
      glUseProgram(comp.program_reference);
      math::Mat4f model =
          math::CreateTranslationMatrix(transform.position) *
          math::CreateRotationMatrix(transform.orientation);
      math::Mat4f matrix = model * view * projection;
      glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &matrix[0]);
      glBindVertexArray(comp.vao_reference);
      glDrawArrays(GL_LINE_LOOP, 0, 10);
    });

    glfw_renderer_.SwapBuffers();
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
  ecs::Entity asteroid_ = 2;
  renderer::GLShaderCache shader_cache_;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();
  return 0;
}
