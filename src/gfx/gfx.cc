#include "gfx.h"

#include "renderer/renderer.cc"

#include "../simulation/command.cc"
#include "../simulation/unit.cc"
#include "../simulation/search.cc"
#include "../simulation/asteroid.cc"
#include "../simulation/tilemap.cc"

namespace gfx
{

constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct Gfx {
  // Allow 32 on screen text messages.
  Text text[kMaxTextCount];
  int text_count;
  rgg::Tag asteroid_tag;
  math::AxisAlignedRect asteroid_aabb;
};

static Gfx kGfx;

bool
Initialize()
{
  int window_result = window::Create("Space", 1280, 720);
  printf("Window create result: %i\n", window_result);
  auto status = rgg::Initialize();
  constexpr int kVertCount = 29;
  constexpr int kFloatCount = kVertCount * 3;
  GLfloat asteroid[kFloatCount] = {
    0.f, 1.6f, 0.f, 0.2f, 1.5f, 0.f, 0.4f, 1.6f, 0.f, 0.6f, 1.6f, 0.f,
    0.68f, 1.9f, 0.f, 1.1f, 1.8f, 0.f, 1.6f, 1.7f, 0.f, 1.8f, 0.9f, 0.f,
    2.3f, 0.3f, 0.f, 2.4f, -0.5f, 0.f, 2.f, -0.8f, 0.f, 1.5f, -1.1f, 0.f,
    0.7f, -1.f, 0.f, 0.5f, -1.1f, 0.f, 0.2f, -1.3f, 0.f, -0.3f, -1.4f, 0.f,
    -1.1f, -1.1f, 0.f, -1.3f, -0.6f, 0.f, -1.25f, -0.2f, 0.f, -1.5f, 0.5f, 0.f,
    -1.4f, 0.4f, 0.f, -1.65f, 1.f, 0.f, -1.6f, 1.3f, 0.f, -1.6f, 1.7f, 0.f,
    -1.4f, 1.9f, 0.f, -1.f, 2.05f, 0.f, -0.7f, 2.07f, 0.f, -0.65f, 2.2f, 0.f,
    -0.5f, 2.25f, 0.f};
  for (int i = 0; i < kFloatCount; ++i) asteroid[i] *= 15.f; // HA
  kGfx.asteroid_aabb.min = math::Vec3f(10000.0f, 10000.0f, 10000.0f);
  kGfx.asteroid_aabb.max = math::Vec3f(-10000.0f, -10000.0f, -10000.0f);
  for (int i = 0; i < kFloatCount; i += 3) {
    auto& aabb = kGfx.asteroid_aabb;
    float x = asteroid[i];
    float y = asteroid[i];
    float z = asteroid[i];
    if (x < aabb.min.x) aabb.min.x = x;
    if (x > aabb.max.x) aabb.max.x = x;
    if (y < aabb.min.y) aabb.min.y = y;
    if (y > aabb.max.y) aabb.max.y = y;
    if (z < aabb.min.z) aabb.min.z = z;
    if (z > aabb.max.z) aabb.max.z = z;
  }
  kGfx.asteroid_tag = rgg::CreateRenderable(kVertCount, asteroid, GL_LINE_LOOP);
  return status;
}

void
Render()
{
  // Draw all text.
  for (int i = 0; i < kGfx.text_count; ++i) {
    Text& text = kGfx.text[i];
    rgg::RenderText(text.msg, text.screen_x, text.screen_y, math::Vec4f());
  }

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
    rgg::RenderRectangle(unit->transform.position, unit->transform.scale,
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
    rgg::RenderRectangle(math::Vec3f(grid),
                         math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                         math::Quatf(0.f, 0.f, 0.f, 1.f), hilite);

    if (unit->command.type == Command::kNone) continue;

    // Show the path they are on if they have one.
    math::Vec2i start = WorldToTilePos(p->xy());
    math::Vec2i end = WorldToTilePos(unit->command.destination);

    auto* path = search::PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    for (int i = 0; i < path->size; ++i) {
      auto* t = &path->tile[i];
      rgg::RenderRectangle(math::Vec3f(TilePosToWorld(*t)),
                           math::Vec3f(1.f / 3.f, 1.f / 3.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f),
                           math::Vec4f(0.33f, 0.33f, 0.66f, 0.7f));
    }
  }

  for (int i = 0; i < kCountAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    rgg::RenderTag(kGfx.asteroid_tag, asteroid->transform.position,
                   asteroid->transform.scale, asteroid->transform.orientation,
                   math::Vec4f(1.f, 1.f, 1.f, 1.f)); 
  }
  
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

      rgg::RenderRectangle(math::Vec3f(TileToWorld(*tile)),
                           math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f), color);
    }
  }

  // TODO: Wny does this stall when vsync is on??
  rgg::RenderGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));
  rgg::RenderGrid(25.f, 25.f, math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f));

  kGfx.text_count = 0;
}

void
PushText(const char* msg, float screen_x, float screen_y)
{
  assert(kGfx.text_count + 1 < kMaxTextCount);
  if (kGfx.text_count + 1 >= kMaxTextCount) return;
  int len = strlen(msg);
  if (len > kMaxTextSize) return;
  Text& text = kGfx.text[kGfx.text_count++];
  strcpy(text.msg, msg);
  text.screen_x = screen_x;
  text.screen_y = screen_y;
}

}  // namespace gfx
