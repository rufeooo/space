#pragma once

#include <set>

#include "asteroids_components.h"

#include "components/common/transform_component.h"
#include "components/rendering/rendering_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "math/vec.h"
#include "renderer/gl_shader_cache.h"
#include "renderer/gl_utils.h"

namespace asteroids {

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
  GLFWwindow* glfw_window = nullptr;
  renderer::GLShaderCache shader_cache;
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
  // ordered_set for determinism when calculating collision.
  std::set<ecs::Entity> projectile_entities;
  std::set<ecs::Entity> asteroid_entities;
  ecs::ComponentStorage<
    component::ViewComponent, PhysicsComponent, PolygonShape,
    component::TransformComponent, InputComponent, GameStateComponent,
    component::RenderingComponent, TTLComponent,
    RandomNumberIntChoiceComponent> components;
};

GameState& GlobalGameState();

ecs::Entity& GlobalFreeEntity();

}