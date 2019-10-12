#pragma once

#include <optional>
#include <set>
#include <vector>

#include "components/common/transform_component.h"
#include "components/rendering/rendering_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "math/vec.h"
#include "renderer/gl_shader_cache.h"
#include "renderer/gl_utils.h"

namespace asteroids {

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
static uint64_t kMaxAsteroidCount = 100;


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

// TODO: Move to components/.
struct PolygonShape {
  PolygonShape() = default;
  PolygonShape(const std::vector<math::Vec2f>& points)
      : points(points) {}
  std::vector<math::Vec2f> points;
};

struct GameStateComponent {
  float seconds_since_last_asteroid_spawn = 0.f;
  uint64_t asteroid_count = 0;
};

struct TTLComponent {
  uint32_t updates_to_live = kProjectileUpdatesToLive;
};

struct RandomNumberIntChoiceComponent {
  RandomNumberIntChoiceComponent() = default;
  RandomNumberIntChoiceComponent(uint8_t number) :
    random_number(number) {}
  uint8_t random_number;
};

// References used for OpenGL.
struct OpenGLGameReferences {
  // Projectiles
  uint32_t projectile_program_reference = 0;
  uint32_t projectile_vao_reference = 0;
  // Asteroids
  uint32_t asteroid_program_reference = 0;
  std::vector<uint32_t> asteroid_vao_references;
  // Ship
  uint32_t ship_program_reference = 0;
  uint32_t ship_vao_reference = 0;
  // Programs
  uint32_t program_reference = 0;
  // Shader uniforms
  uint32_t matrix_uniform_location = 0;
};

// Used for manipulating OpenGl state.
struct OpenGL {
  ecs::Entity camera = 0;
  GLFWwindow* glfw_window = nullptr;
  renderer::GLShaderCache shader_cache;
  OpenGLGameReferences game_references;
};

// Geometry of game objects. Specifically the vertices that make
// up the game objects in local space.
struct EntityGeometry {
  std::vector<math::Vec2f> ship_geometry;
  std::vector<math::Vec2f> projectile_geometry;
  std::vector<std::vector<math::Vec2f>> asteroid_geometry;
};

struct GameState {
  // ordered_set for determinism when calculating collision.
  std::set<ecs::Entity> projectile_entities;
  std::set<ecs::Entity> asteroid_entities;
  ecs::ComponentStorage<
    component::ViewComponent, PhysicsComponent, PolygonShape,
    component::TransformComponent, InputComponent, GameStateComponent,
    component::RenderingComponent, TTLComponent,
    RandomNumberIntChoiceComponent> components;
};

struct Options {
  ecs::Entity free_entity = 1;
  EntityGeometry entity_geometry;
  // Set if a user would like asteroids to render. A server may not.
  std::optional<OpenGL> opengl;
  GameState game_state;
};

ecs::Entity SpawnAsteroid(
    Options& options, const math::Vec3f& position, math::Vec3f dir,
    float angle, int random_number=-1);

ecs::Entity SpawnPlayer(Options& options, const math::Vec3f& position);

bool Initialize(Options& options);

void ProcessClientInput(Options& options);

void UpdateGame(Options& options);

bool RenderGame(Options& options);

}
