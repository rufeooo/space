#include "search.cc"

namespace gameplay {

bool
Initialize()
{
  if (!gfx::Initialize()) return false;
  {
    auto* transform = &game_entity[0].transform;
    transform->position = math::Vec3f(300.f, 300.f, 0.f);
    transform->scale = math::Vec3f(0.25f, 0.25f, 0.f);
  }

  camera::MoveTo(math::Vec3f(400.f, 400.f, 0.f));
  tilemap::Initialize();

  return true;
}

bool
Update()
{
  using namespace tilemap;
  gfx::ResetRenderData();
  auto sz = window::GetWindowSize();
  char buffer[50];
  sprintf(buffer, "Frame Time:%06lu us", kGameState.frame_time_usec);
  gfx::PushText(buffer, 3.f, sz.y);
  sprintf(buffer, "Window Size:%ix%i", (int)sz.x, (int)sz.y);
  gfx::PushText(buffer, 3.f, sz.y - 25.f);
  auto mouse = camera::ScreenToWorldSpace(window::GetCursorPosition());
  sprintf(buffer, "Mouse Pos In World:(%.1f,%.1f)", mouse.x, mouse.y);
  gfx::PushText(buffer, 3.f, sz.y - 50.f);

  {
    Entity* ent = &game_entity[0];
    math::Vec3f* p = &ent->transform.position;
    math::Vec2f grid = TilePosToWorld(WorldToTilePos(p->xy()));

    // Draw the player.
    gfx::PushRectangle(ent->transform.position, ent->transform.scale,
                       ent->transform.orientation,
                       math::Vec4f(0.26f, 0.33f, 0.78f, 1.f));

    // Highlight the tile the player is on.
    gfx::PushRectangle(math::Vec3f(grid),
                         math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                         math::Quatf(0.f, 0.f, 0.f, 1.f),
                         math::Vec4f(1.f, 0.f, 0.f, .45f)); 
  }

  gfx::PushGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));
  // TODO: Wny does this stall??
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

  if (!COMPONENT_EXISTS(0, destination)) return false;
  Entity* ent = &game_entity[0];
  DestinationComponent* destination = &ent->destination;
  TransformComponent* transform = &ent->transform;
  if (destination) {
    math::Vec2i start = WorldToTilePos(transform->position.xy());
    math::Vec2i end = WorldToTilePos(destination->position);

    sprintf(buffer, "(%i,%i) to (%i, %i)", start.x, start.y, end.x, end.y); 
    gfx::PushText(buffer, 3.f, 30.f);

    auto* path = search::PathTo(start, end);
    if (path && path->size > 1) {
      for (int i = 0; i < path->size; ++i) {
        auto* t = &path->tile[i];
        gfx::PushRectangle(math::Vec3f(TilePosToWorld(*t)),
                           math::Vec3f(1.f / 3.f, 1.f / 3.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f),
                           math::Vec4f(0.33f, 0.33f, 0.66f, 0.7f)); 
      }
      math::Vec3f dest = TilePosToWorld(path->tile[1]);
      auto dir = math::Normalize(dest - transform->position.xy());
      transform->position += dir * 1.f;
    }
  }

  return true;
}

}
