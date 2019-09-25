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

namespace {

//////// Ship Constants //////// 
// Seconds until the ship reaches max speed.
static float kSecsToMaxSpeed = 3.f;
// TODO: Convert this to seconds until velocity dampens to 0.
static float kDampenVelocity = 0.00001f;
// How fast the ship accelerates.
static float kShipAcceleration = 0.00004f;
// Calculation for seconds for the ship to fully rotate.
//     15 ms an update (see game.h)
//     ms till full speed = kSecsToFullRotation * 1000.f
//     rotation degrees a game loop = ms till full speed / 360.f
static float kSecsToFullRotation = 1.5f;
static float kRotationSpeed =
    (1000.f * kSecsToFullRotation) / 360.f;

//////// Projectile Constants //////// 
// Calcuation for a projectile to live n seconds.
//     15 ms an update (see game.h)
//     (n * 1000) / 15
//     (4 * 1000) / 15 = 266.6 = to int -> 266
// TODO: Convert this to seconds for projectile to live.
static int kProjectileUpdatesToLive = 266;
// Initial / max speed of projectile.
static float kProjectileSpeed = 0.005;

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
  // Set to true when the user request a projectile to be shot.
  bool shoot_projectile = false;
  // State the space bar was last in.
  int space_state;
  // If a was pressed in the past process input calls.
  bool a_pressed = false; 
  // If b was pressed in the past process input calls.
  bool d_pressed = false; 
};

struct PhysicsComponent {
  PhysicsComponent() = default;
  PhysicsComponent(
      const math::Vec3f& acceleration,
      const math::Vec3f& velocity,
      float acceleration_speed,
      float max_velocity) :
    acceleration(acceleration), velocity(velocity),
    acceleration_speed(acceleration_speed),
    max_velocity(max_velocity) {}
  math::Vec3f acceleration;
  math::Vec3f velocity;
  float acceleration_speed = kShipAcceleration;
  float max_velocity = kShipAcceleration * kSecsToMaxSpeed * 60.f;
};

struct AsteroidComponent {
  uint32_t vao_reference;
  uint32_t program_reference;
};

struct BulletComponent {
  BulletComponent() = default;
  BulletComponent(uint32_t vao_reference, uint32_t program_reference) :
    vao_reference(vao_reference),
    program_reference(program_reference) {}
  uint32_t vao_reference;
  uint32_t program_reference;
};

struct TTLComponent {
  uint32_t updates_to_live = kProjectileUpdatesToLive;
};

void SpawnPlayerProjectile(
    ecs::Entity entity,
    const component::TransformComponent& transform,
    uint32_t vao_reference,
    uint32_t program_reference) {
  auto orientation = transform.orientation;
  auto dir = orientation.Up();
  dir.Normalize();
  component::TransformComponent projectile_transform(transform);
  projectile_transform.position += (dir * .08f);
  ecs::Assign<component::TransformComponent>(
      entity, projectile_transform);
  ecs::Assign<PhysicsComponent>(
      entity, math::Vec3f(), dir * kProjectileSpeed, 0.f, 0.f);
  ecs::Assign<BulletComponent>(
      entity, vao_reference, program_reference);
  ecs::Assign<TTLComponent>(entity);
}

}  // namespace

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
    projectile_vao_reference_ = renderer::CreateGeometryVAO({
        0.f, 0.005f, 0.f,
        0.005f, 0.0f, 0.f,
        0.f, -0.005f, 0.f,
        -0.005f, 0.0f, 0.f
    });
    projectile_program_reference_ = program_reference;
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
        input.a_pressed = true;
      }
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_D);
      if (state == GLFW_PRESS) {
        input.d_pressed = true;
      }
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
      state = glfwGetKey(glfw_renderer_.window(), GLFW_KEY_SPACE);
      // If the current state of space was release and the previous
      // was pressed then set the ship to fire.
      if (state == GLFW_RELEASE && input.space_state == GLFW_PRESS) {
        input.shoot_projectile = true;
      }
      input.space_state = state;
    });
    return true;
  }

  // Game logic
  bool Update() override {
    // Provide ship control to the entity with Input (the player.)
    ecs::Enumerate<PhysicsComponent, component::TransformComponent,
                   InputComponent>(
        [this](ecs::Entity ent, PhysicsComponent& physics,
               component::TransformComponent& transform,
               InputComponent& input) {
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
      if (input.shoot_projectile) {
        SpawnPlayerProjectile(
            free_entity_++, transform, projectile_vao_reference_,
            projectile_program_reference_);
        input.shoot_projectile = false;
      }
      if (input.a_pressed) {
        transform.orientation.Rotate(-kRotationSpeed);
        input.a_pressed = false;
      }
      if (input.d_pressed) {
        transform.orientation.Rotate(kRotationSpeed);
        input.d_pressed = false;
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

    std::vector<ecs::Entity> ents_to_kill;
    ecs::Enumerate<TTLComponent>([&ents_to_kill](
        ecs::Entity ent, TTLComponent& ttl) {
      --ttl.updates_to_live;
      if (!ttl.updates_to_live) ents_to_kill.push_back(ent);
    });

    for (const auto& e : ents_to_kill) {
      ecs::Remove<component::TransformComponent>(e);
      ecs::Remove<PhysicsComponent>(e);
      ecs::Remove<BulletComponent>(e);
      ecs::Remove<TTLComponent>(e);
    }

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
    ecs::Enumerate<BulletComponent,
                   component::TransformComponent>(
        [&](ecs::Entity ent,
            BulletComponent& comp,
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
    glfw_renderer_.SwapBuffers();
    char title[256];
    sprintf(
        title, "Asteroids MS Per Frame: %lld FPS: %.2f",
        ms_per_frame().count(), fps());
    glfwSetWindowTitle(glfw_renderer_.window(), title);
    return true;
  }

 private:
  int matrix_location_;
  ecs::Entity camera_ = 0;
  ecs::Entity player_ = 1;
  ecs::Entity asteroid_ = 2;
  ecs::Entity free_entity_ = 3;
  renderer::GLShaderCache shader_cache_;
  uint32_t projectile_vao_reference_;
  uint32_t projectile_program_reference_;
};

int main() {
  Asteroids asteroids;
  asteroids.Run();
  return 0;
}
