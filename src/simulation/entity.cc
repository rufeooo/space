
#include "common/common.cc"
#include "math/math.cc"

// Data used by game simulation
#define DECLARE_GAME_TYPE(type, count) DECLARE_ARRAY(type, count)
#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

struct Transform {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
};

struct Asteroid {
  Transform transform;
};
DECLARE_GAME_TYPE(Asteroid, 8);

struct Command {
  enum Type {
    kNone = 0,
    kMine = 1,
    kMove = 2,
  };
  Type type;
  math::Vec2f destination;
};

DECLARE_GAME_QUEUE(Command, 16);
struct Unit {
  Transform transform;
  Command command;
  uint64_t think_flags = 0;
  int kind = 0;
};

DECLARE_GAME_TYPE(Unit, 8);

struct Ship {
  uint64_t satisfied_flags = 0;
};
DECLARE_GAME_TYPE(Ship, 1);

