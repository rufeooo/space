#pragma once

#include <optional>
#include <set>
#include <vector>

#include "components/common/transform_component.h"
#include "components/rendering/rendering_component.h"
#include "components/rendering/view_component.h"
#include "ecs/ecs.h"
#include "game/event.h"
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

bool Initialize();

void HandleEvent(game::Event event);

bool UpdateGame();

bool RenderGame();

}
