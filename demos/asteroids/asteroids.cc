#include "asteroids.h"

#include <iostream>
#include <random>

#include "ecs/ecs.h"
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

bool InitializeGraphics(Options& options) {
  auto& opengl = *options.opengl;
  auto& components = options.game_state.components;
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
      kProgramName, &opengl.game_references.program_reference)) {
    return false;
  }
  opengl.game_references.matrix_uniform_location =
      glGetUniformLocation(opengl.game_references.program_reference,
                           "matrix");
  opengl.game_references.projectile_program_reference =
      opengl.game_references.program_reference;
  opengl.game_references.asteroid_program_reference =
      opengl.game_references.program_reference;
  opengl.game_references.ship_program_reference =
      opengl.game_references.program_reference;
  std::cout << opengl.shader_cache.GetProgramInfo(kProgramName)
            << std::endl;
  return true;
}

void CreateAsteroidGeometry(
      const std::vector<math::Vec2f>& geometry, float scale,
      Options& options) {
  std::vector<math::Vec2f> scaled_geometry;
  for (const auto& g : geometry) {
    scaled_geometry.push_back(g / scale);
  }
  options.entity_geometry.asteroid_geometry.push_back(scaled_geometry);
  if (options.opengl) {
    options.opengl->game_references.asteroid_vao_references.push_back(
        renderer::CreateGeometryVAO(scaled_geometry));
  }
}

ecs::Entity SpawnPlayer(Options& options,
                        const math::Vec3f& position) {
  auto& components = options.game_state.components;
  components.Assign<PhysicsComponent>(options.free_entity);
  components.Assign<PolygonShape>(options.free_entity,
                            options.entity_geometry.ship_geometry);
  components.Assign<component::TransformComponent>(options.free_entity);
  if (options.opengl) {
    components.Assign<component::RenderingComponent>(
      options.free_entity,
      options.opengl->game_references.ship_vao_reference,
      options.opengl->game_references.program_reference,
      options.entity_geometry.ship_geometry.size());
  }
  ++options.free_entity;
  return options.free_entity - 1;
}

void SpawnPlayerProjectile(
    Options& options,
    const component::TransformComponent& transform) {
  auto& components = options.game_state.components;
  auto& orientation = transform.orientation;
  auto dir = orientation.Up();
  dir.Normalize();
  component::TransformComponent projectile_transform(transform);
  projectile_transform.position += (dir * .08f);
  components.Assign<component::TransformComponent>(
      options.free_entity, projectile_transform);
  components.Assign<PhysicsComponent>(
      options.free_entity, math::Vec3f(), dir * kProjectileSpeed, 0.f,
      0.f);
  components.Assign<TTLComponent>(options.free_entity);
  components.Assign<PolygonShape>(
      options.free_entity,
      options.entity_geometry.projectile_geometry);
  if (options.opengl) {
    components.Assign<component::RenderingComponent>(
        options.free_entity,
        options.opengl->game_references.projectile_vao_reference,
        options.opengl->game_references.program_reference,
        options.entity_geometry.projectile_geometry.size());
  }
  ++options.free_entity;
}

ecs::Entity SpawnAsteroid(
    Options& options, const math::Vec3f& position, math::Vec3f dir,
    float angle, int random_number) {
  auto& components = options.game_state.components;
  dir.Normalize();
  components.Assign<component::TransformComponent>(options.free_entity);
  auto* transform = components.Get<component::TransformComponent>(
      options.free_entity);
  transform->position = position;
  transform->orientation.Set(angle, math::Vec3f(0.f, 0.f, 1.f));
  components.Assign<PhysicsComponent>(options.free_entity);
  auto* physics = components.Get<PhysicsComponent>(options.free_entity);
  physics->velocity = dir * kShipAcceleration * 50.f;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<>
      disi(0, options.entity_geometry.asteroid_geometry.size() - 1);
  if (random_number == -1) {
    random_number = disi(gen);
  }
  // Store off the random number that was used to create the asteroid.
  // This is useful for server->client communication when the client
  // needs to recreate the asteroid.
  components.Assign<RandomNumberIntChoiceComponent>(
      options.free_entity, (uint8_t)random_number);
  components.Assign<PolygonShape>(
      options.free_entity,
      options.entity_geometry.asteroid_geometry[random_number]);
  if (options.opengl) {
    components.Assign<component::RenderingComponent>(
        options.free_entity,
        options.opengl->game_references
            .asteroid_vao_references[random_number],
        options.opengl->game_references.program_reference,
        options.entity_geometry
            .asteroid_geometry[random_number].size());
  }
  options.game_state.asteroid_entities.insert(options.free_entity);
  ++options.free_entity;
  return options.free_entity - 1;
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
    Options& options, ecs::Entity projectile, ecs::Entity asteroid) {
  auto& components = options.game_state.components;
  auto* projectile_transform
      = components.Get<component::TransformComponent>(projectile);
  auto* projectile_physics = components.Get<PhysicsComponent>(projectile);
  auto* asteroid_transform
      = components.Get<component::TransformComponent>(asteroid);
  auto* asteroid_shape = components.Get<PolygonShape>(asteroid);
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

bool Initialize(Options& options) {
  if (options.opengl) {
    if (!InitializeGraphics(options)) return false;
  }
  // Create ship geometry.
  // Create ship vao if OpenGL is provide.
  options.entity_geometry.ship_geometry =  {
    {0.0f, 0.08f}, {0.03f, -0.03f}, {0.00f, -0.005f},
    {-0.03f, -0.03f}
  };
  if (options.opengl) {
    options.opengl->game_references.ship_vao_reference =
        renderer::CreateGeometryVAO(
            options.entity_geometry.ship_geometry);
  }

  // For all asteroids:
  //   Create asteroid geometry.
  //   Create asteroid vao if OpenGL is provided.
  CreateAsteroidGeometry({
      {0.0f, 0.1f}, {0.07f, 0.08f}, {0.06f, -0.01f}, {0.11f, -0.005f},
      {0.1f, -0.06f}, {0.05f, -0.08f}, {0.01f, -0.1f},
      {-0.07f, -0.08f}, {-0.1f, -0.01f}, {-0.08f, 0.06f},
    }, 1.0f, options);

  CreateAsteroidGeometry({
    {0.f, 2.f}, {0.5f, 2.1f}, {0.9f, 1.9f}, {1.9f, 1.85f},
    {2.6f, 1.65f}, {3.f, 1.f}, {2.93f, 0.1f}, {2.f, -1.f},
    {1.5f, -1.4f}, {0.5f, -1.2f}, {0.f, -1.f}, {-1.1f, -0.95f},
    {-1.7f, -0.7f}, {-2.f, 0.f}, {-1.f, 1.f}, {-1.f, 1.5f},
    {-0.5f, 1.6f}
  }, 20.f, options);

  CreateAsteroidGeometry({
    {0.f, 1.6f}, {0.2f, 1.5f}, {0.4f, 1.6f}, {0.6f, 1.6f},
    {0.68f, 1.9f}, {1.1f, 1.8f}, {1.6f, 1.7f}, {1.8f, 0.9f},
    {2.3f, 0.3f}, {2.4f, -0.5f}, {2.f, -0.8f}, {1.5f, -1.1f},
    {0.7f, -1.f}, {0.5f, -1.1f}, {0.2f, -1.3f}, {-0.3f, -1.4f},
    {-1.1f, -1.1f}, {-1.3f, -0.6f}, {-1.25f, -0.2f}, {-1.5f, 0.5f},
    {-1.4f, 0.4f}, {-1.65f, 1.f}, {-1.6f, 1.3f}, {-1.6f, 1.7f},
    {-1.4f, 1.9f}, {-1.f, 2.05f}, {-0.7f, 2.07f}, {-0.65f, 2.2f},
    {-0.5f, 2.25f}
  }, 20.f, options);
  
  // Create projectile geometry.
  // Create projectile vao if OpenGL is provided.
  options.entity_geometry.projectile_geometry =  {
      {0.f, 0.005f}, {0.005f, 0.0f}, {0.f, -0.005f}, {-0.005f, 0.0}
  };
  if (options.opengl) {
    options.opengl->game_references.projectile_vao_reference =
        renderer::CreateGeometryVAO(
            options.entity_geometry.projectile_geometry);
  }

  return true;
}

void ProcessClientInput(Options& options) {
  auto& opengl = *options.opengl;
  auto& components = options.game_state.components;
  glfwPollEvents();
  components.Enumerate<InputComponent, PhysicsComponent,
                       component::TransformComponent>(
      [&](ecs::Entity ent, InputComponent& input,
                PhysicsComponent& physics,
                component::TransformComponent& transform) {
    // Apply rotation. 
    int state = glfwGetKey(opengl.glfw_window, GLFW_KEY_A);
    if (state == GLFW_PRESS) {
      input.a_pressed = true;
    }
    state = glfwGetKey(opengl.glfw_window, GLFW_KEY_D);
    if (state == GLFW_PRESS) {
      input.d_pressed = true;
    }
    // Set acceleration to facing direction times acceleration_speed.
    auto u = transform.orientation.Up();
    state = glfwGetKey(opengl.glfw_window, GLFW_KEY_W);
    physics.acceleration = math::Vec3f(0.f, 0.f, 0.f);
    if (state == GLFW_PRESS) {
      physics.acceleration = u * physics.acceleration_speed;
    }
    state = glfwGetKey(opengl.glfw_window, GLFW_KEY_S);
    if (state == GLFW_PRESS) {
      physics.acceleration = u * -physics.acceleration_speed;
    }
    state = glfwGetKey(opengl.glfw_window, GLFW_KEY_SPACE);
    // If the current state of space was release and the previous
    // was pressed then set the ship to fire.
    if (state == GLFW_RELEASE && input.space_state == GLFW_PRESS) {
      input.shoot_projectile = true;
    }
    input.space_state = state;
  });
}

void UpdateGame(Options& options) {
  auto& components = options.game_state.components;
  std::set<ecs::Entity> asteroids_to_kill;
  std::set<ecs::Entity> projectiles_to_kill;
  // Do collision at the top of the loop so the player has seen the
  // most recent positions collision detection is calculating
  // against. Otherwise it seems like collision is happening a frame
  // in the future.
  for (const auto& projectile :
      options.game_state.projectile_entities) {
    for (const auto& asteroid :
        options.game_state.asteroid_entities) {
      if (ProjectileCollidesWithAsteroid(
          options, projectile, asteroid)) {
        asteroids_to_kill.insert(asteroid);
        projectiles_to_kill.insert(projectile);
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
    components.Remove<component::TransformComponent>(e);
    components.Remove<PhysicsComponent>(e);
    components.Remove<PolygonShape>(e);
    components.Remove<component::RenderingComponent>(e);
    components.Remove<TTLComponent>(e);
    options.game_state.projectile_entities.erase(e);
  }

  for (const auto& e : asteroids_to_kill) {
    components.Remove<component::TransformComponent>(e);
    components.Remove<PhysicsComponent>(e);
    components.Remove<PolygonShape>(e);
    components.Remove<component::RenderingComponent>(e);
    components.Remove<RandomNumberIntChoiceComponent>(e);
    options.game_state.asteroid_entities.erase(e);
  }

  // Provide ship control to the entity with Input (the player.)
  components.Enumerate<PhysicsComponent, component::TransformComponent,
                       InputComponent>(
      [&options](ecs::Entity ent, PhysicsComponent& physics,
                 component::TransformComponent& transform,
                 InputComponent& input) {
    UpdatePhysics(physics);
    if (input.shoot_projectile) {
      options.game_state.projectile_entities.insert(
          options.free_entity);
      SpawnPlayerProjectile(options, transform);
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
    //math::Vec3f rotated_local_point =
    //math::CreateRotationMatrix(transform.orientation)
    //std::cout << transform.position.String() << std::endl;
    if (max_x <= -1.0f) {
      //std::cout << "max x 1" << std::endl;
      //std::cout << -min_x + 0.99f << std::endl;
      transform.position.x() = .99f;
    } else if (min_x >= 1.0f) {
      //std::cout << "max x 2" << std::endl;
      transform.position.x() = -0.99f;
    }
    if (max_y <= -1.0f) { 
      //std::cout << max_y << " <= 1.0f" << std::endl;
      transform.position.y() = 0.99f;
    } else if (min_y >= 1.0f) {
      //std::cout << min_y << " >= 1.0f" << std::endl;
      transform.position.y() = -0.99f;
    }
  });

  components.Enumerate<GameStateComponent>(
      [&options](ecs::Entity ent, GameStateComponent& game_state) {
    game_state.seconds_since_last_asteroid_spawn += 15.f / 1000.0f;
    if (game_state.seconds_since_last_asteroid_spawn >=
        kSecsToSpawnAsteroid &&
        game_state.asteroid_count < kMaxAsteroidCount) {
      // TODO: Better random number generation.
      static std::random_device rd;
      static std::mt19937 gen(rd());
      static std::uniform_real_distribution<>
          disr(-10000.0, 10000.0);
            options.game_state.asteroid_entities.insert(
                options.free_entity);
      SpawnAsteroid(
          options,
          math::Vec3f(disr(gen), disr(gen), 0.f),
          math::Vec3f(disr(gen), disr(gen), 0.f),
          disr(gen));
      game_state.seconds_since_last_asteroid_spawn = 0.f;
      game_state.asteroid_count++;
    }
  });
}

bool RenderGame(Options& options) {
  auto& opengl = *options.opengl;
  auto& components = options.game_state.components;
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
        opengl.game_references.matrix_uniform_location, 1, GL_FALSE,
        &matrix[0]);
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
