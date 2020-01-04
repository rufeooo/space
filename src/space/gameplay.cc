namespace gameplay {

bool
Initialize()
{
  if (!gfx::Initialize()) return false;
  {
    auto* transform = &game_entity[0].transform;
    transform->position = math::Vec3f(0.f, 0.f, 0.f);
    transform->scale = math::Vec3f(0.25f, 0.25f, 0.f);
  }

  camera::MoveTo(math::Vec3f(400.f, 400.f, 0.f));
  tilemap::Create();

  return true;

}

bool
Update()
{
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
    math::Vec2f grid = {
      (p->x - fmod(p->x, tilemap::kTileWidth)) + tilemap::kTileWidth / 2.f,
      (p->y - fmod(p->y, tilemap::kTileHeight)) + tilemap::kTileHeight / 2.f};

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
  gfx::PushGrid(25.f, 25.f, math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f));

  for (int i = 0; i < tilemap::kMapHeight; ++i) {
    for (int j = 0; j < tilemap::kMapWidth; ++j) {
      using namespace tilemap;
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
    auto dir =
        math::Normalize(destination->position - transform->position.xy());
    transform->position += dir * 1.f;
    float length_squared =
        math::LengthSquared(transform->position.xy() - destination->position);
    // Remove DestinationComponent so entity stops.
    if (length_squared < 15.0f) {
      COMPONENT_RESET(0, destination);
    }
  }

  return true;
}

}
