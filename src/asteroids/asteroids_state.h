#pragma once
#include <set>
#include <vector>

#include "asteroids_components.h"

#include "components/common/input_component.h"
#include "components/common/transform_component.h"
#include "components/network/server_authoritative_component.h"
#include "components/rendering/rendering_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "math/vec.h"
#include "gl/shader_cache.h"
#include "gl/utils.h"

namespace asteroids
{
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

OpenGLGameReferences& GlobalOpenGLGameReferences();

// Used for manipulating OpenGl state.
struct OpenGL {
  ecs::Entity camera = 0;
  gl::ShaderCache shader_cache;
};

OpenGL& GlobalOpenGL();

// Geometry of game objects. Specifically the vertices that make
// up the game objects in local space.
struct EntityGeometry {
  std::vector<math::Vec2f> ship_geometry;
  std::vector<math::Vec2f> projectile_geometry;
  std::vector<std::vector<math::Vec2f>> asteroid_geometry;
};

EntityGeometry& GlobalEntityGeometry();

struct GameState {
  ecs::ComponentStorage<
      ViewComponent, PhysicsComponent, PolygonShape,
      TransformComponent, InputComponent,
      GameStateComponent, RenderingComponent, TTLComponent,
      AsteroidComponent, PlayerComponent, ProjectileComponent,
      RandomNumberIntChoiceComponent, ServerAuthoritativeComponent>
      components;
  ecs::Entity player_id = -1;
};

GameState& GlobalGameState();

// For entity generation.
void SetEntityStart(ecs::Entity entity);
void SetEntityIncrement(int increment);
ecs::Entity GenerateFreeEntity();

}  // namespace asteroids
