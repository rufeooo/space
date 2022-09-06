
#include "math/math.cc"

#include "tilemap.cc"

namespace simulation
{
Rectf
ShipBounds(uint64_t ship_index)
{
  Rectf bounds;
  if (ship_index >= kUsedShip) return bounds;

  Ship* ship = &kShip[ship_index];
  bounds.x = ship->transform.position.x;
  bounds.y = ship->transform.position.y;
  bounds.width = kMapWidth * kTileWidth;
  bounds.height = kMapHeight * kTileHeight;

  return bounds;
}

Rectf
FleetBounds()
{
  // Find min and max bounds of all grids.
  v2f min(FLT_MAX, FLT_MAX);
  v2f max_dims(0.0f, 0.0f);
  for (int i = 0; i < kUsedShip; ++i) {
    Rectf bounds = ShipBounds(i);

    min.x = fmin(bounds.x, min.x);
    min.y = fmin(bounds.y, min.y);

    max_dims.x = fmax(bounds.width, max_dims.x);
    max_dims.y = fmax(bounds.height, max_dims.y);
  }

  return Rectf(min.x, min.y, max_dims.x, max_dims.y);
}

}  // namespace simulation
