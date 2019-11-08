#include "asteroids_state.h"

#include <iostream>

namespace asteroids {

OpenGLGameReferences& GlobalOpenGLGameReferences() {
  static OpenGLGameReferences opengl_game_references;
  return opengl_game_references;
}

OpenGL& GlobalOpenGL() {
  static OpenGL opengl;
  return opengl;
}

EntityGeometry& GlobalEntityGeometry() {
  static EntityGeometry entity_geometry;
  return entity_geometry;
}

GameState& GlobalGameState() {
  static GameState game_state;
  return game_state;
}

ecs::Entity& GlobalFreeEntity() {
  static ecs::Entity free_entity = 1;
  return free_entity;
}

}
