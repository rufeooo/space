#include "search.cc"
#include "asteroid.cc"
#include "unit.cc"

namespace simulation
{
bool
Initialize()
{
  math::Vec3f pos[] = {
      math::Vec3f(300.f, 300.f, 0.f), math::Vec3f(100.f, 130.f, 0),
      math::Vec3f(300.f, 400.f, 0), math::Vec3f(650.f, 500.f, 0)};
  const math::Vec3f scale = math::Vec3f(0.25f, 0.25f, 0.f);
  for (int i = 0; i < ARRAY_LENGTH(pos); ++i) {
    Unit* unit = NewUnit();
    unit->transform.position = pos[i];
    unit->destination = pos[i].xy();
    unit->transform.scale = scale;
  }

  Asteroid* asteroid = NewAsteroid();
  asteroid->transform.position = math::Vec3f(400.f, 750.f, 0.f);

  tilemap::Initialize();

  return true;
}

bool
VerifyIntegrity()
{
  // TODO (AN): Checksum of kEntity compared to end of last frame
  return true;
}

void
Update()
{
  using namespace tilemap;

  for (int i = 0; i < kCountUnit; ++i) {
    Unit* unit = &kUnit[i];
    Transform* transform = &unit->transform;

    math::Vec2i start = WorldToTilePos(transform->position.xy());
    math::Vec2i end = WorldToTilePos(unit->destination);

    auto* path = search::PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    math::Vec3f dest = TilePosToWorld(path->tile[1]);
    auto dir = math::Normalize(dest - transform->position.xy());
    transform->position += dir * 1.f;
  }

  for (int i = 0; i < kCountAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    asteroid->transform.position.x -= 1.0f;
    if (asteroid->transform.position.x < 0.f) {
      asteroid->transform.position.x = 800.f;
    }
  }
}

}  // namespace simulation
