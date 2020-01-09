#include "entity.cc"
#include "search.cc"

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
    kWriteEntity[i].transform.position = pos[i];
    kWriteEntity[i].transform.scale = scale;
  }
  kWriteEntity[0].kind = 1;
  // Apply default state to kReadEntity
  EntityAdvance();

  tilemap::Initialize();

  return true;
}

bool
VerifyIntegrity()
{
  return memcmp(kWriteEntity, kReadEntity, sizeof(kReadEntity)) == 0;
}

void
Update()
{
  using namespace tilemap;

  const Entity* ent_end = kWriteEntity + MAX_ENTITY;
  for (Entity* ent = kWriteEntity; ent < ent_end; ++ent) {
    if (!COMPONENT_EXISTS(ent, destination)) continue;
    TransformComponent* transform = &ent->transform;

    math::Vec2i start = WorldToTilePos(transform->position.xy());
    math::Vec2i end = WorldToTilePos(ent->destination.position);

    auto* path = search::PathTo(start, end);
    if (!path || path->size <= 1) {
      COMPONENT_RESET(ent, destination);
      continue;
    }

    math::Vec3f dest = TilePosToWorld(path->tile[1]);
    auto dir = math::Normalize(dest - transform->position.xy());
    transform->position += dir * 1.f;
  }
}

void
ToRenderer()
{
  using namespace tilemap;

  const Entity* ent_end = kReadEntity + MAX_ENTITY;

  for (const Entity* ent = kReadEntity; ent < ent_end; ++ent) {
    if (!EntityExists(ent)) continue;

    const math::Vec3f* p = &ent->transform.position;
    math::Vec2f grid = TilePosToWorld(WorldToTilePos(p->xy()));

    math::Vec4f color;
    switch (ent->kind) {
      case 1:
        color = math::Vec4f(0.26f, 0.33f, 0.78f, 1.f);
        break;
      default:
        color = math::Vec4f(1.0, .4, .4, 1.f);
        break;
    }
    // Draw the player.
    gfx::PushRectangle(ent->transform.position, ent->transform.scale,
                       ent->transform.orientation, color);

    math::Vec4f hilite;
    switch (ent->kind) {
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
  }

  gfx::PushGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));
  // TODO: Wny does this stall when vsync is on??
  gfx::PushGrid(25.f, 25.f, math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f));

  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = &kTilemap.map[i][j];
      if (tile->type == NONE) continue;
      gfx::PushRectangle(math::Vec3f(TileToWorld(*tile)),
                         math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                         math::Quatf(0.f, 0.f, 0.f, 1.f),
                         math::Vec4f(1.f, 1.f, 1.f, 1.f));
    }
  }

  for (const Entity* ent = kReadEntity; ent <= kReadEntity; ++ent) {
    if (!COMPONENT_EXISTS(ent, destination)) continue;

    math::Vec2i start = WorldToTilePos(ent->transform.position.xy());
    math::Vec2i end = WorldToTilePos(ent->destination.position);

    char buffer[50];
    sprintf(buffer, "(%i,%i) to (%i, %i)", start.x, start.y, end.x, end.y);
    gfx::PushText(buffer, 3.f, 30.f);
  }

  for (const Entity* ent = kReadEntity; ent < ent_end; ++ent) {
    if (!COMPONENT_EXISTS(ent, destination)) continue;
    const DestinationComponent* destination = &ent->destination;
    const TransformComponent* transform = &ent->transform;

    math::Vec2i start = WorldToTilePos(transform->position.xy());
    math::Vec2i end = WorldToTilePos(destination->position);

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
}

void
ApplyUpdate()
{
  EntityAdvance();
}

}  // namespace simulation
