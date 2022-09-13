#pragma once

#include "common/common.c"

#include "renderer/renderer.cc"

#include "imui.cc"

namespace gfx
{
struct Gfx {
  RenderTag cryo_tag;
  RenderTag exhaust_tag;
  RenderTag plus_tag;
};

static Gfx kGfx;
static v4f kSelectionColor = v4f(0.19f, 0.803f, 0.19f, 0.40f);
static v4f kSelectionOutlineColor = v4f(0.19f, 0.803f, 0.19f, 1.f);
static v4f kRed = v4f(1.f, 0.f, 0.f, 1.f);
static v4f kGreen = v4f(0.f, 1.f, 0.f, 1.f);
static v4f kBlue = v4f(0.f, 0.f, 1.f, 1.f);
static v4f kWhite = v4f(1.f, 1.f, 1.f, 1.f);
static v4f kGray = v4f(.6f, .6f, .6f, 1.f);
static const math::Quatf kDefaultRotation = math::Quatf(0.f, 0.f, 0.f, 1.f);
static v3f kDefaultScale = v3f(1.f, 1.f, 1.f);
static v3f kTileScale = v3f(0.5f, 0.5f, 1.f);
static bool kRenderGrid = false;
static bool kRenderPath = false;

struct DebugCube {
  math::Cubef cube;
  v4f color;
};

DECLARE_ARRAY(DebugCube, 64);

bool
Initialize(int fullscreen, Rect* window)
{
  int window_result = platform_init("space", fullscreen, window);
  printf("Window create result: %i\n", window_result);
  auto status = rgg::Initialize();
  constexpr int kVertCount = 29;
  constexpr int kFloatCount = kVertCount * 3;
  // clang-format off
  constexpr int kCryoVert = 6;
  GLfloat cryo[kCryoVert*3] = 
  {-1.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f,
   1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
   0.0f, 3.0f, 0.f, -1.0f, 1.0f, 0.0f };
  constexpr int kExhaustVert = 12;
  GLfloat exhaust[kExhaustVert*3] = 
  {0.0f, 3.0f, 0.0f, 3.0f, 3.0f, 0.0f,
   4.0f, 2.7f, 0.0f, 3.5f, 2.4f, 0.0f,
   4.2f, 2.1f, 0.0f, 3.5f, 1.8f, 0.0f,
   4.2f, 1.5f, 0.0f, 3.5f, 1.2f, 0.0f,
   4.2f, 0.9f, 0.0, 3.5f, 0.6f, 0.0f,
   3.0, 0.3f, 0.0, 0.0f, 0.0f, 0.0f};
  constexpr int kPlusVert = 8;
#define ORIGIN 0.0f, 0.0f, 0.0f
  GLfloat plus[kPlusVert*3] = 
  {-1.0f, 0.f, 0.f, ORIGIN,
    0.0f, -1.0f, 0.0f, ORIGIN,
    1.0f, 0.0f, 0.0f, ORIGIN, 
    0.0f, 1.0f, 0.0f, ORIGIN};
  // clang-format on
  for (int i = 0; i < kCryoVert * 3; ++i) cryo[i] *= 12.f;        // HA
  for (int i = 0; i < kExhaustVert * 3; ++i) exhaust[i] *= 15.f;  // HA
  for (int i = 0; i < kPlusVert * 3; ++i) plus[i] *= 15.f;        // HA
  kGfx.cryo_tag = rgg::CreateRenderable(kCryoVert, cryo, GL_LINE_LOOP);
  kGfx.exhaust_tag = rgg::CreateRenderable(kExhaustVert, exhaust, GL_LINE_LOOP);
  kGfx.plus_tag = rgg::CreateRenderable(kPlusVert, plus, GL_LINE_LOOP);
  return status;
}

void
Reset()
{
  kUsedDebugCube = 0;
}

void
PushDebugCube(const math::Cubef& cube, const v4f& color)
{
  DebugCube* dcube = UseDebugCube();
  dcube->cube = cube;
  dcube->color = color;
}

void
RenderDebugCube()
{
  // Render debug graphics.
  for (int i = 0; i < kUsedDebugCube; ++i) {
    rgg::RenderLineCube(kDebugCube[i].cube, kDebugCube[i].color);
  }
}

}  // namespace gfx
