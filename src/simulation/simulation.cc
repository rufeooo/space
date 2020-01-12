#include "search.cc"
#include "asteroid.cc"
#include "unit.cc"

#include "../gfx/gfx.cc"

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

void
ToRenderer()
{
  using namespace tilemap;

  for (int i = 0; i < kCountUnit; ++i) {
    Unit* unit = &kUnit[i];

    const math::Vec3f* p = &unit->transform.position;
    math::Vec2f grid = TilePosToWorld(WorldToTilePos(p->xy()));

    math::Vec4f color;
    switch (unit->kind) {
      case 1:
        color = math::Vec4f(0.26f, 0.33f, 0.78f, 1.f);
        break;
      default:
        color = math::Vec4f(1.0, .4, .4, 1.f);
        break;
    }
    // Draw the player.
    gfx::PushRectangle(unit->transform.position, unit->transform.scale,
                       unit->transform.orientation, color);

    math::Vec4f hilite;
    switch (unit->kind) {
      case 1:
        hilite = math::Vec4f(1.f, 0.f, 0.f, .45f);
        break;
      default:
        hilite = math::Vec4f(5.f, 5.f, 5.f, .45f);
        break;
    };
    // Highlight the tile the player is on.
    gfx::PushRectangle(math::Vec3f(grid),
                       math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                       math::Quatf(0.f, 0.f, 0.f, 1.f), hilite);

    // Show the path they are on if they have one.
    math::Vec2i start = WorldToTilePos(p->xy());
    math::Vec2i end = WorldToTilePos(unit->destination);

    auto* path = search::PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    for (int i = 0; i < path->size; ++i) {
      auto* t = &path->tile[i];
      gfx::PushRectangle(math::Vec3f(TilePosToWorld(*t)),
                         math::Vec3f(1.f / 3.f, 1.f / 3.f, 1.f),
                         math::Quatf(0.f, 0.f, 0.f, 1.f),
                         math::Vec4f(0.33f, 0.33f, 0.66f, 0.7f));
    }
  }

  for (int i = 0; i < kCountAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    gfx::PushAsteroid(asteroid->transform.position, asteroid->transform.scale,
                      asteroid->transform.orientation,
                      math::Vec4f(1.f, 1.f, 1.f, 1.f)); 
  }

  // TODO: Wny does this stall when vsync is on??
  gfx::PushGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));
  gfx::PushGrid(25.f, 25.f, math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f));

  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = &kTilemap.map[i][j];
      uint64_t type_id = tile->type;

      if (type_id == NONE) continue;

      math::Vec4f color;
      switch (type_id) {
        case 1:
          color = math::Vec4f(1.f, 1.f, 1.f, 1.f);
          break;
        case 2:
          color = math::Vec4f(1.0f, 0.0f, 0.f, 1.0f);
          break;
        case 3:
          color = math::Vec4f(0.0f, 0.0f, 0.75f, 1.0f);
          break;
        case 4:
          color = math::Vec4f(0.0, 0.75f, 0.0f, 1.0f);
      };

      gfx::PushRectangle(math::Vec3f(TileToWorld(*tile)),
                         math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                         math::Quatf(0.f, 0.f, 0.f, 1.f), color);
    }
  }
}

}  // namespace simulation
