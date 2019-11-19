#include "asteroids.h"

#include <iostream>
#include <random>

#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "components/network/server_authoritative_component.h"
#include "ecs/ecs.h"
#include "game/game.h"
#include "game/event_buffer.h"
#include "math/intersection.h"
#include "math/mat_ops.h"
#include "math/vec.h"
#include "renderer/gl_utils.h"

#include "ecs/internal.h"

namespace asteroids {

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

bool InitializeGraphics() {
  auto& opengl = GlobalOpenGL();
  auto& components = GlobalGameState().components;
  opengl.glfw_window = renderer::InitGLAndCreateWindow(
      800, 800, "Asteroids");
  if (!opengl.glfw_window) {
    std::cout << "Unable to start GL and create window."
              << std::endl;
    return false;
  }
  components.Assign<component::ViewComponent>(
    opengl.camera,
    math::Vec3f(0.0f, 0.0f, 1.5f),
    math::Quatf(0.0f, math::Vec3f(0.0f, 0.0f, -1.0f)));
  if (!opengl.shader_cache.CompileShader(
      kVertexShaderName,
      renderer::ShaderType::VERTEX,
      kVertexShader)) {
    std::cout << "Unable to compile "
              << kVertexShaderName << std::endl;
    return false;
  }
  if (!opengl.shader_cache.CompileShader(
      kFragmentShaderName,
      renderer::ShaderType::FRAGMENT,
      kFragmentShader)) {
    std::cout << "Unable to compile "
              << kFragmentShaderName << std::endl;
    return false;
  }
  if (!opengl.shader_cache.LinkProgram(
      kProgramName,
      {kVertexShaderName, kFragmentShaderName})) {
    std::cout << "Unable to link: "
              << kProgramName
              << " info: "
              << opengl.shader_cache.GetProgramInfo(kProgramName)
              << std::endl;
    return false;
  }
  if (!opengl.shader_cache.GetProgramReference(
      kProgramName, &GlobalOpenGLGameReferences().program_reference)) {
    return false;
  }
  GlobalOpenGLGameReferences().matrix_uniform_location =
      glGetUniformLocation(
          GlobalOpenGLGameReferences().program_reference, "matrix");
  GlobalOpenGLGameReferences().projectile_program_reference =
      GlobalOpenGLGameReferences().program_reference;
  GlobalOpenGLGameReferences().asteroid_program_reference =
      GlobalOpenGLGameReferences().program_reference;
  GlobalOpenGLGameReferences().ship_program_reference =
      GlobalOpenGLGameReferences().program_reference;
  std::cout << opengl.shader_cache.GetProgramInfo(kProgramName)
            << std::endl;
  return true;
}

void CreateAsteroidGeometry(
      const std::vector<math::Vec2f>& geometry, float scale) {
  std::vector<math::Vec2f> scaled_geometry;
  for (const auto& g : geometry) {
    scaled_geometry.push_back(g / scale);
  }
  GlobalEntityGeometry().asteroid_geometry.push_back(scaled_geometry);
  GlobalOpenGLGameReferences().asteroid_vao_references.push_back(
      renderer::CreateGeometryVAO(scaled_geometry));
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
  auto& components = GlobalGameState().components;
  auto* projectile_transform
      = components.Get<component::TransformComponent>(projectile);
  auto* projectile_physics = components.Get<PhysicsComponent>(projectile);
  auto* asteroid_transform
      = components.Get<component::TransformComponent>(asteroid);
  auto* asteroid_shape = components.Get<PolygonShape>(asteroid);
  assert(projectile_physics != nullptr);
  assert(asteroid_transform != nullptr);
  assert(asteroid_shape != nullptr);
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

bool Initialize() {
  if (!InitializeGraphics()) return false;
  // Create ship geometry.
  // Create ship vao if OpenGL is provide.
  GlobalEntityGeometry().ship_geometry =  {
    {0.0f, 0.08f}, {0.03f, -0.03f}, {0.00f, -0.005f},
    {-0.03f, -0.03f}
  };
  GlobalOpenGLGameReferences().ship_vao_reference =
      renderer::CreateGeometryVAO(GlobalEntityGeometry().ship_geometry);

  // For all asteroids:
  //   Create asteroid geometry.
  //   Create asteroid vao if OpenGL is provided.
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
  
  // Create projectile geometry.
  // Create projectile vao if OpenGL is provided.
  GlobalEntityGeometry().projectile_geometry =  {
      {0.f, 0.005f}, {0.005f, 0.0f}, {0.f, -0.005f}, {-0.005f, 0.0}
  };
  GlobalOpenGLGameReferences().projectile_vao_reference =
      renderer::CreateGeometryVAO(
          GlobalEntityGeometry().projectile_geometry);
  return true;
}

void HandleEvent(game::Event event) {
  switch ((Event)event.metadata) {
    case Event::CREATE_PLAYER:
      commands::Execute(*((CreatePlayer*)event.data));
      break;
    case Event::CREATE_ASTEROID:
      commands::Execute(*((CreateAsteroid*)event.data));
      break;
    case Event::CREATE_PROJECTILE:
      commands::Execute(*((CreateProjectile*)event.data));
      break;
    default:
      assert("Event is unhandled.");
  }
}

bool UpdateGame() {
  auto& components = GlobalGameState().components;
  std::set<ecs::Entity> asteroids_to_kill;
  std::set<ecs::Entity> projectiles_to_kill;
  // Do collision at the top of the loop so the player has seen the
  // most recent positions collision detection is calculating
  // against. Otherwise it seems like collision is happening a frame
  // in the future.
  for (const auto& projectile : GlobalGameState().projectile_entities) {
    for (const auto& asteroid : GlobalGameState().asteroid_entities) {
      if (ProjectileCollidesWithAsteroid(
          projectile.entity, asteroid.entity)) {
        asteroids_to_kill.insert(asteroid.entity);
        projectiles_to_kill.insert(projectile.entity);
        break;
      }
    }
  }

  components.Enumerate<TTLComponent>([&projectiles_to_kill](
      ecs::Entity ent, TTLComponent& ttl) {
    --ttl.updates_to_live;
    if (!ttl.updates_to_live) projectiles_to_kill.insert(ent);
  });

  for (const auto& e : projectiles_to_kill) {
    components.Delete(e);
    for (int i = 0;
         i < GlobalGameState().projectile_entities.size(); ++i) {
      auto& projectile_data = GlobalGameState().projectile_entities[i];
      if (projectile_data.entity == e) {
        GlobalGameState().projectile_entities.erase(
          GlobalGameState().projectile_entities.begin() + i);
        break;
      }
    }
  }

  for (const auto& e : asteroids_to_kill) {
    components.Delete(e);
    for (int i = 0;
         i < GlobalGameState().asteroid_entities.size(); ++i) {
      auto& asteroid_data = GlobalGameState().asteroid_entities[i];
      if (asteroid_data.entity == e) {
        GlobalGameState().asteroid_entities.erase(
          GlobalGameState().asteroid_entities.begin() + i);
        break;
      }
    }
  }

  // Provide ship control to the entity with Input (the player.)
  components.Enumerate<PhysicsComponent, component::TransformComponent,
                       component::InputComponent>(
      [](ecs::Entity ent, PhysicsComponent& physics,
         component::TransformComponent& transform,
         component::InputComponent& input) {
    UpdatePhysics(physics);
    // TODO: Make sure this doesn't happen too often???
    if (component::IsKeyDown(
          input.previous_input_mask, component::KEYBOARD_SPACE) &&
        component::IsKeyUp(
          input.input_mask, component::KEYBOARD_SPACE)) {
      auto* create_projectile = game::CreateEvent<asteroids::CreateProjectile>(
          Event::CREATE_PROJECTILE);
      create_projectile->mutate_entity_id(GenerateFreeEntity());
      auto& projectile_transform = create_projectile->mutable_transform();
      projectile_transform.mutable_position() =
          asteroids::Vec3(transform.position.x(),
                          transform.position.y(),
                          transform.position.z());
      projectile_transform.mutable_orientation() =
          asteroids::Vec4(transform.orientation.x(),
                          transform.orientation.y(),
                          transform.orientation.z(),
                          transform.orientation.w());
      projectile_transform.mutable_prev_position() =
           asteroids::Vec3(transform.prev_position.x(),
                           transform.prev_position.y(),
                           transform.prev_position.z());
    }
    if (component::IsKeyDown(
        input.input_mask, component::KEYBOARD_A)) {
      transform.orientation.Rotate(-kRotationSpeed);
    }
    if (component::IsKeyDown(
        input.input_mask, component::KEYBOARD_D)) {
      transform.orientation.Rotate(kRotationSpeed);
    }
    auto u = transform.orientation.Up();
    physics.acceleration = math::Vec3f(0.f, 0.f, 0.f);
    if (component::IsKeyDown(
        input.input_mask, component::KEYBOARD_W)) {
      physics.acceleration = u * physics.acceleration_speed;
    }
    if (component::IsKeyDown(
        input.input_mask, component::KEYBOARD_S)) {
      physics.acceleration = u * -physics.acceleration_speed;
    }
  });

  components.Enumerate<PhysicsComponent, component::TransformComponent,
                       PolygonShape>(
      [](ecs::Entity ent, PhysicsComponent& physics,
         component::TransformComponent& transform,
         PolygonShape& shape) {
    transform.prev_position = transform.position;
    transform.position += physics.velocity;
    // Get entity min/max x and y.
    auto world_transform =
        math::CreateTranslationMatrix(transform.position) *
        math::CreateRotationMatrix(transform.orientation);
    float min_x = 10000.f, max_x = -10000.f,
          min_y = 10000.f, max_y = -10000.f;
    int min_x_idx, max_x_idx, min_y_idx, max_y_idx;
    for (int i = 0; i < shape.points.size(); ++i) {
      const auto& point = shape.points[i];
      math::Vec3f world_point = 
        world_transform * math::Vec3f(point.x(), point.y(), 0.f);
      if (world_point.x() < min_x) {
        min_x = world_point.x();
        min_x_idx = i;
      }
      if (world_point.x() > max_x) {
        max_x = world_point.x();
        max_x_idx = i;
      }
      if (world_point.y() < min_y) {
        min_y = world_point.y();
        min_y_idx = i;
      }
      if (world_point.y() > max_y) {
        max_y = world_point.y();
        max_y_idx = i;
      }
    }
    bool teleported = false;
    if (max_x <= -1.0f) {
      transform.position.x() = .99f;
      teleported = true;
    } else if (min_x >= 1.0f) {
      transform.position.x() = -0.99f;
      teleported = true;
    }
    if (max_y <= -1.0f) { 
      transform.position.y() = 0.99f;
      teleported = true;
    } else if (min_y >= 1.0f) {
      transform.position.y() = -0.99f;
      teleported = true;
    }
    if (teleported) {
      // Teleporting across the screen causes collision detection to draw
      // a line across the screen when detecting hits. Force the prev position
      // to be something more reasonable if the entity teleports.
      transform.prev_position = transform.position;
    }
  });

  components.Enumerate<GameStateComponent>(
      [](ecs::Entity ent, GameStateComponent& game_state) {
    game_state.seconds_since_last_asteroid_spawn += 15.f / 1000.0f;
    if (game_state.seconds_since_last_asteroid_spawn >=
        kSecsToSpawnAsteroid &&
        game_state.asteroid_count < kMaxAsteroidCount) {
      // TODO: Better random number generation.
      static std::random_device rd;
      static std::mt19937 gen(rd());
      static std::uniform_real_distribution<>
          disr(-10000.0, 10000.0);
      auto* create_asteroid = game::CreateEvent<CreateAsteroid>(
          Event::CREATE_ASTEROID);
      create_asteroid->mutate_entity_id(GenerateFreeEntity());
      create_asteroid->mutable_position() =
          asteroids::Vec3(disr(gen), disr(gen), 0.f);
      create_asteroid->mutable_direction() =
          asteroids::Vec3(disr(gen), disr(gen), 0.f);
      create_asteroid->mutate_angle(disr(gen));
      create_asteroid->mutate_random_number(0);
      game_state.seconds_since_last_asteroid_spawn = 0.f;
      game_state.asteroid_count++;
    }
  });

  return true;
}

bool RenderGame() {
  auto& opengl = GlobalOpenGL();
  auto& components = GlobalGameState().components;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  auto* view_component = components.Get<component::ViewComponent>(
        opengl.camera);
  math::Mat4f view = math::CreateViewMatrix(
      view_component->position, view_component->orientation);
  math::Mat4f projection =
        math::CreatePerspectiveMatrix<float>(800, 800);
  auto projection_view = projection * view;
  components.Enumerate<component::RenderingComponent,
                       component::TransformComponent>(
      [&](ecs::Entity ent,
          component::RenderingComponent& comp,
          component::TransformComponent& transform) {
    glUseProgram(comp.program_reference);
    math::Mat4f model =
        math::CreateTranslationMatrix(transform.position) *
        math::CreateRotationMatrix(transform.orientation);
    math::Mat4f matrix = projection_view * model;
    glUniformMatrix4fv(
        GlobalOpenGLGameReferences().matrix_uniform_location, 1,
        GL_FALSE, &matrix[0]);
    glBindVertexArray(comp.vao_reference);
    glDrawArrays(GL_LINE_LOOP, 0, comp.vertex_count);
  });
  glfwSwapBuffers(opengl.glfw_window);
  /*char title[256];
  sprintf(
      title, "Asteroids MS Per Frame: %lld FPS: %.2f",
      ms_per_frame().count(), fps());
  glfwSetWindowTitle(opengl.glfw_window, title);*/
  return !glfwWindowShouldClose(opengl.glfw_window);
}

}
