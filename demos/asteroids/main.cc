#include <iostream>
#include <set>
#include <random>
#include <thread>

#include <gflags/gflags.h>

#include "components/common/transform_component.h"
#include "components/rendering/rendering_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "game/game.h"
#include "math/intersection.h"
#include "math/mat_ops.h"
#include "math/vec.h"
#include "network/client.h"
#include "network/message_queue.h"
#include "network/server.h"
#include "renderer/gl_shader_cache.h"
#include "renderer/gl_utils.h"

DEFINE_bool(server, false, "Set to true to run as server.");
DEFINE_string(hostname, "",
              "If provided will connect to a game server.");
DEFINE_string(port, "1843", "Port for this application.");

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

//////// Game Constants //////// 
static float kSecsToSpawnAsteroid = 2.3f;
static uint64_t kMaxAsteroidCount = 0xFFFFFFFF;

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

struct TTLComponent {
  uint32_t updates_to_live = kProjectileUpdatesToLive;
};

struct GameStateComponent {
  float seconds_since_last_asteroid_spawn = 0.f;
  uint64_t asteroid_count = 0;
};

struct PolygonShape {
  PolygonShape() = default;
  PolygonShape(const std::vector<math::Vec2f>& points)
      : points(points) {}
  std::vector<math::Vec2f> points;
};

void SpawnPlayer(
    ecs::Entity entity, const math::Vec3f& position,
    uint32_t vao_reference, uint32_t program_reference,
    const std::vector<math::Vec2f>& ship_geometry) {
  ecs::Assign<InputComponent>(entity);
  ecs::Assign<PhysicsComponent>(entity);
  ecs::Assign<PolygonShape>(entity, ship_geometry);
  ecs::Assign<component::TransformComponent>(entity);
  ecs::Assign<component::RenderingComponent>(
      entity, vao_reference, program_reference,
      ship_geometry.size());
}

void SpawnPlayerProjectile(
    ecs::Entity entity, const component::TransformComponent& transform,
    uint32_t vao_reference, uint32_t program_reference) {
  auto orientation = transform.orientation;
  auto dir = orientation.Up();
  dir.Normalize();
  component::TransformComponent projectile_transform(transform);
  projectile_transform.position += (dir * .08f);
  ecs::Assign<component::TransformComponent>(
      entity, projectile_transform);
  ecs::Assign<PhysicsComponent>(
      entity, math::Vec3f(), dir * kProjectileSpeed, 0.f, 0.f);
  ecs::Assign<component::RenderingComponent>(
      entity, vao_reference, program_reference, 4);
  ecs::Assign<TTLComponent>(entity);
}

void SpawnAsteroid(
    ecs::Entity entity, const math::Vec3f& position,
    math::Vec3f dir, float angle, uint32_t vao_reference,
    uint32_t program_reference,
    const std::vector<math::Vec2f>& asteroid_geometry) {
  dir.Normalize();
  ecs::Assign<component::TransformComponent>(entity);
  auto* transform = ecs::Get<component::TransformComponent>(entity);
  transform->position = position;
  transform->orientation.Set(angle, math::Vec3f(0.f, 0.f, 1.f));
  ecs::Assign<PhysicsComponent>(entity);
  auto* physics = ecs::Get<PhysicsComponent>(entity);
  physics->velocity = dir * kShipAcceleration * 50.f;
  ecs::Assign<PolygonShape>(entity, asteroid_geometry);
  ecs::Assign<component::RenderingComponent>(
      entity, vao_reference, program_reference,
      asteroid_geometry.size());
}

void UpdatePhysics(PhysicsComponent& physics_component) {
  // If the ship is not at max velocity and the ship has
  // acceleration.
  auto velocity_squared
      = math::LengthSquared(physics_component.velocity);
  if (velocity_squared < 
        physics_component.max_velocity *
        physics_component.max_velocity &&
      math::LengthSquared(physics_component.acceleration) > 0.f) {
    physics_component.velocity += physics_component.acceleration;
  } else if (math::LengthSquared(physics_component.velocity) > 0.f) {
    // Dampen velocity.
    auto vdir = physics_component.velocity;
    vdir.Normalize();
    physics_component.velocity -= vdir * kDampenVelocity;
    if (velocity_squared < kShipAcceleration * kShipAcceleration) {
      physics_component.velocity = math::Vec3f(0.f, 0.f, 0.f);
    }
  }
}

bool ProjectileCollidesWithAsteroid(
    const math::Vec2f& projectile_start,
    const math::Vec2f& projectile_end,
    const std::vector<math::Vec2f>& asteroid_shape_points,
    const math::Vec3f& asteroid_position,
    const math::Quatf& asteroid_orientation) {
  // Generate asteroid line list in world coordinates.
  std::vector<math::Vec2f> asteroid_points = asteroid_shape_points;
  auto asteroid_transform =
      math::CreateTranslationMatrix(asteroid_position) *
      math::CreateRotationMatrix(asteroid_orientation);
  // Offset the points relative to the asteroids transform.
  for (auto& point : asteroid_points) {
    math::Vec3f p_3d_transformed =
        asteroid_transform * math::Vec3f(point.x(), point.y(), 0.f);
    point = math::Vec2(p_3d_transformed.x(), p_3d_transformed.y());
  }
  // Check if the line created by the moving projectile intersects
  // any line created by the points of the asteroid.
  for (int i = 0; i < asteroid_points.size(); ++i) {
    math::Vec2f point_start(asteroid_points[i].x(),
                            asteroid_points[i].y());
    int end_idx = (i + 1) % asteroid_points.size();
    math::Vec2f point_end(
        asteroid_points[end_idx].x(), asteroid_points[end_idx].y());
    if (math::LineSegmentsIntersect(projectile_start, projectile_end,
                                    point_start, point_end,
                                    nullptr, nullptr)) {
      return true;
    }
  }
  return false;
}

bool ProjectileCollidesWithAsteroid(
    ecs::Entity projectile, ecs::Entity asteroid) {
  auto* projectile_transform
      = ecs::Get<component::TransformComponent>(projectile);
  auto* projectile_physics = ecs::Get<PhysicsComponent>(projectile);
  auto* asteroid_transform
      = ecs::Get<component::TransformComponent>(asteroid);
  auto* asteroid_shape = ecs::Get<PolygonShape>(asteroid);
  math::Vec2f projectile_start(projectile_transform->prev_position.x(),
                               projectile_transform->prev_position.y());
  math::Vec2f projectile_end(projectile_transform->position.x(),
                             projectile_transform->position.y());
  // Check if the line made by the projectile will intersect the
  // asteroid at its current position.
  if (ProjectileCollidesWithAsteroid(
      projectile_start, projectile_end, asteroid_shape->points,
      asteroid_transform->position, asteroid_transform->orientation)) {
    return true;
  }

  // Also check the previous game updates asteroid location. This
  // will solve the case where the asteroid and projectile are going
  // in opposite directions and a line test will never actually
  // intersect.
  if (ProjectileCollidesWithAsteroid(
      projectile_start, projectile_end, asteroid_shape->points,
      asteroid_transform->prev_position,
      asteroid_transform->orientation)) {
    return true;
  }

  return false;
}

}  // namespace

class Asteroids : public game::Game {
 public:
  Asteroids() : game::Game() {};
  bool Initialize() override {
    if (FLAGS_server) {
      network_thread_ = network::server::Create(
          FLAGS_port.c_str(), &outgoing_message_queue_,
          &incoming_message_queue_);
    } else if (!FLAGS_hostname.empty()) {
      network_thread_ = network::client::Create(
          FLAGS_hostname.c_str(), FLAGS_port.c_str(),
          &outgoing_message_queue_, &incoming_message_queue_);
    }
    glfw_window_ = renderer::InitGLAndCreateWindow(
        800, 800, "Asteroids");
    if (!glfw_window_) {
      std::cout << "Unable to start GL and create window."
                << std::endl;
      return false;
    }
    ecs::Assign<component::ViewComponent>(
      camera_,
      math::Vec3f(0.0f, 0.0f, 1.5f),
      math::Quatf(0.0f, math::Vec3f(0.0f, 0.0f, -1.0f)));
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
    std::cout << shader_cache_.GetProgramInfo(kProgramName)
              << std::endl;

    // Create ship geometry.

    std::vector<math::Vec2f> ship_geometry = {
        {0.0f, 0.08f}, {0.03f, -0.03f}, {0.00f, -0.005f},
        {-0.03f, -0.03f}
    };
    uint32_t vao_ship_reference =
        renderer::CreateGeometryVAO(ship_geometry);

    // Create game state component.
    
    ecs::Assign<GameStateComponent>(free_entity_++);

    // Create player.

    SpawnPlayer(free_entity_++, math::Vec3f(0.f, 0.f, 0.f),
                vao_ship_reference, program_reference, ship_geometry);

    // Create asteroid geometry and save its vao / program ref.

    CreateAsteroidGeometry({
      {0.0f, 0.1f}, {0.07f, 0.08f}, {0.06f, -0.01f}, {0.11f, -0.005f},
      {0.1f, -0.06f}, {0.05f, -0.08f}, {0.01f, -0.1f},
      {-0.07f, -0.08f}, {-0.1f, -0.01f}, {-0.08f, 0.06f},
    }, 1.0f);

    CreateAsteroidGeometry({
      {0.f, 2.f}, {0.5f, 2.1f}, {0.9f, 1.9f}, {1.9f, 1.85f},
      {2.6f, 1.65f}, {3.f, 1.f}, {2.93f, 0.1f}, {2.f, -1.f},
      {1.5f, -1.4f}, {0.5f, -1.2f}, {0.f, -1.f}, {-1.1f, -0.95f},
      {-1.7f, -0.7f}, {-2.f, 0.f}, {-1.f, 1.f}, {-1.f, 1.5f},
      {-0.5f, 1.6f}
    }, 20.f);

    CreateAsteroidGeometry({
      {0.f, 1.6f}, {0.2f, 1.5f}, {0.4f, 1.6f}, {0.6f, 1.6f},
      {0.68f, 1.9f}, {1.1f, 1.8f}, {1.6f, 1.7f}, {1.8f, 0.9f},
      {2.3f, 0.3f}, {2.4f, -0.5f}, {2.f, -0.8f}, {1.5f, -1.1f},
      {0.7f, -1.f}, {0.5f, -1.1f}, {0.2f, -1.3f}, {-0.3f, -1.4f},
      {-1.1f, -1.1f}, {-1.3f, -0.6f}, {-1.25f, -0.2f}, {-1.5f, 0.5f},
      {-1.4f, 0.4f}, {-1.65f, 1.f}, {-1.6f, 1.3f}, {-1.6f, 1.7f},
      {-1.4f, 1.9f}, {-1.f, 2.05f}, {-0.7f, 2.07f}, {-0.65f, 2.2f},
      {-0.5f, 2.25f}
    }, 20.f);


    asteroid_program_reference_ = program_reference;

    // Create projectile geometry and save its vao / program ref.
    
    std::vector<math::Vec2f> projectile_geometry = {
      {0.f, 0.005f}, {0.005f, 0.0f}, {0.f, -0.005f}, {-0.005f, 0.0}
    };
    projectile_vao_reference_
        = renderer::CreateGeometryVAO(projectile_geometry);
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
      int state = glfwGetKey(glfw_window_, GLFW_KEY_A);
      if (state == GLFW_PRESS) {
        input.a_pressed = true;
      }
      state = glfwGetKey(glfw_window_, GLFW_KEY_D);
      if (state == GLFW_PRESS) {
        input.d_pressed = true;
      }
      // Set acceleration to facing direction times acceleration_speed.
      auto u = transform.orientation.Up();
      state = glfwGetKey(glfw_window_, GLFW_KEY_W);
      physics.acceleration = math::Vec3f(0.f, 0.f, 0.f);
      if (state == GLFW_PRESS) {
        physics.acceleration = u * physics.acceleration_speed;
      }
      state = glfwGetKey(glfw_window_, GLFW_KEY_S);
      if (state == GLFW_PRESS) {
        physics.acceleration = u * -physics.acceleration_speed;
      }
      state = glfwGetKey(glfw_window_, GLFW_KEY_SPACE);
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
    std::set<ecs::Entity> asteroids_to_kill;
    std::set<ecs::Entity> projectiles_to_kill;
    // Do collision at the top of the loop so the player has seen the
    // most recent positions collision detection is calculating
    // against. Otherwise it seems like collision is happening a frame
    // in the future.
    for (const auto& projectile : projectile_entities_) {
      for (const auto& asteroid : asteroid_entities_) {
        if (ProjectileCollidesWithAsteroid(projectile, asteroid)) {
          asteroids_to_kill.insert(asteroid);
          projectiles_to_kill.insert(projectile);
          break;
        }
      }
    }

    ecs::Enumerate<TTLComponent>([&projectiles_to_kill](
        ecs::Entity ent, TTLComponent& ttl) {
      --ttl.updates_to_live;
      if (!ttl.updates_to_live) projectiles_to_kill.insert(ent);
    });

    for (const auto& e : projectiles_to_kill) {
      ecs::Remove<component::TransformComponent>(e);
      ecs::Remove<PhysicsComponent>(e);
      ecs::Remove<PolygonShape>(e);
      ecs::Remove<component::RenderingComponent>(e);
      ecs::Remove<TTLComponent>(e);
      projectile_entities_.erase(e);
    }

    for (const auto& e : asteroids_to_kill) {
      ecs::Remove<component::TransformComponent>(e);
      ecs::Remove<PhysicsComponent>(e);
      ecs::Remove<PolygonShape>(e);
      ecs::Remove<component::RenderingComponent>(e);
      asteroid_entities_.erase(e);
    }

    // Provide ship control to the entity with Input (the player.)
    ecs::Enumerate<PhysicsComponent, component::TransformComponent,
                   InputComponent>(
        [this](ecs::Entity ent, PhysicsComponent& physics,
               component::TransformComponent& transform,
               InputComponent& input) {
      UpdatePhysics(physics);

      if (input.shoot_projectile) {
        projectile_entities_.insert(free_entity_);
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
      transform.prev_position = transform.position;
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

    ecs::Enumerate<GameStateComponent>(
        [&](ecs::Entity ent, GameStateComponent& game_state) {
      game_state.seconds_since_last_asteroid_spawn +=
          ms_per_update() / 1000.0f;
      if (game_state.seconds_since_last_asteroid_spawn >=
          kSecsToSpawnAsteroid &&
          game_state.asteroid_count < kMaxAsteroidCount) {
        // TODO: Better random number generation.
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<>
            disr(-10000.0, 10000.0);
        static std::uniform_int_distribution<>
            disi(0, asteroid_vao_reference_.size() - 1);
        asteroid_entities_.insert(free_entity_);
        int idx = disi(gen);
        SpawnAsteroid(
            free_entity_++, math::Vec3f(disr(gen), disr(gen), 0.f),
            math::Vec3f(disr(gen), disr(gen), 0.f), disr(gen),
            asteroid_vao_reference_[idx], asteroid_program_reference_,
            asteroid_geometry_[idx]);
        game_state.seconds_since_last_asteroid_spawn = 0.f;
        game_state.asteroid_count++;
      }
    });

    return true;
  }

  // Render logic
  bool Render() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto* view_component = ecs::Get<component::ViewComponent>(
          camera_);
    math::Mat4f view = math::CreateViewMatrix(
        view_component->position, view_component->orientation);
    math::Mat4f projection =
          math::CreatePerspectiveMatrix<float>(800, 800);
    auto projection_view = projection * view;
    ecs::Enumerate<component::RenderingComponent,
                   component::TransformComponent>(
        [&](ecs::Entity ent,
            component::RenderingComponent& comp,
            component::TransformComponent& transform) {
      glUseProgram(comp.program_reference);
      math::Mat4f model =
          math::CreateTranslationMatrix(transform.position) *
          math::CreateRotationMatrix(transform.orientation);
      math::Mat4f matrix = projection_view * model;
      glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &matrix[0]);
      glBindVertexArray(comp.vao_reference);
      glDrawArrays(GL_LINE_LOOP, 0, comp.vertex_count);
    });
    glfwSwapBuffers(glfw_window_);
    char title[256];
    sprintf(
        title, "Asteroids MS Per Frame: %lld FPS: %.2f",
        ms_per_frame().count(), fps());
    glfwSetWindowTitle(glfw_window_, title);
    return !glfwWindowShouldClose(glfw_window_);
  }

  void OnGameEnd() override {
    if (network_thread_.joinable()) {
      outgoing_message_queue_.Stop();
      network_thread_.join();
    }
  }

 private:
  void CreateAsteroidGeometry(
      const std::vector<math::Vec2f>& geometry, float scale) {
    std::vector<math::Vec2f> scaled_geometry;
    for (const auto& g : geometry) {
      scaled_geometry.push_back(g / scale);
    }
    asteroid_geometry_.push_back(scaled_geometry);
    asteroid_vao_reference_.push_back(
        renderer::CreateGeometryVAO(scaled_geometry));
  }

  int matrix_location_;
  ecs::Entity camera_ = 0;
  ecs::Entity free_entity_ = 1;
  GLFWwindow* glfw_window_;
  renderer::GLShaderCache shader_cache_;
  uint32_t projectile_vao_reference_;
  uint32_t projectile_program_reference_;
  std::vector<uint32_t> asteroid_vao_reference_;
  uint32_t asteroid_program_reference_;
  std::vector<std::vector<math::Vec2f>> asteroid_geometry_;
  // ordered_set for determinism when calculating collision.
  std::set<ecs::Entity> projectile_entities_;
  std::set<ecs::Entity> asteroid_entities_;
  // queues to send / recv messages.
  network::OutgoingMessageQueue outgoing_message_queue_;
  network::IncomingMessageQueue incoming_message_queue_;
  std::thread network_thread_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  Asteroids asteroids;
  asteroids.Run();
  return 0;
}
