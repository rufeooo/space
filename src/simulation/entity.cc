#pragma once

#include "math/math.cc"

#include "camera.cc"
#include "entity_registry.cc"

// Data used by game simulation
#define DECLARE_GAME_TYPE(type, max_count)                               \
  DECLARE_ARRAY(type, max_count)                                         \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type, \
                                    max_count, sizeof(type), nullptr,    \
                                    nullptr);

// kInvalidId is reserved for invalid references
#define DECLARE_GAME_TYPE_WITH_ID(type, max_count)                             \
  DECLARE_HASH_ARRAY(type, max_count)                                          \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type,       \
                                    max_count, sizeof(type), kHashEntry##type, \
                                    Hash##type);

#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

// Use with DECLARE_GAME_TYPE_WITH_ID(Entity, ...)
#define DECLARE_GAME_ENTITY(type, tid)           \
  type* UseEntity##type()                        \
  {                                              \
    Entity* e = UseEntity();                     \
    if (!e) return nullptr;                      \
    type t = {};                                 \
    t.id = e->id;                                \
    memcpy(e, &t, sizeof(t));                    \
    return (type*)e;                             \
  }                                              \
                                                 \
  type* i2##type(uint64_t idx)                   \
  {                                              \
    assert(idx < kUsedEntity);                   \
    type* t = (type*)(&kEntity[idx]);            \
    if (t->type_id != tid) return nullptr;       \
    if (t->id == 0) return nullptr;              \
    return t;                                    \
  }                                              \
                                                 \
  type* Find##type(uint32_t id)                  \
  {                                              \
    type* t = (type*)FindEntity(id);             \
    if (!t || t->type_id != tid) return nullptr; \
    return t;                                    \
  }

#define FOR_EACH_ENTITY(type, vname, body)                          \
  {                                                                 \
    type* vname;                                                    \
    for (int Idx##type = 0; Idx##type < kUsedEntity; ++Idx##type) { \
      vname = i2##type(Idx##type);                                  \
      if (!vname) continue;                                         \
      body                                                          \
    }                                                               \
  }

// Global game state that is local information
static uint64_t kPlayerCount;
static uint64_t kPlayerIndex;

// Common Flags
enum UnitAction {
  kUaNone = 0,
  kUaMove,
  kUaOperate,
  kUaAttack,
  kUaAttackMove,
  kUaBuild,
};
enum ModuleKind {
  kModPower = 0,
  kModEngine,
  kModTurret,
  kModMine,
  kModDoor,
  kModBarrack,
  kModMedbay,
  kModWarp,
  kModCount,
};
enum UnitKind {
  kOperator,
  kAlien,
};
enum HudMode {
  kHudSelection = 0,
  kHudModule,
  kHudAttackMove,
  kHudDefault = kHudSelection,
};
enum WeaponKind {
  kWeaponLaser = 0,
  kWeaponBullet,
  kWeaponMiningLaser,
  kWeaponCount,
};
constexpr unsigned kHudModeBits = 2;
constexpr unsigned kModBits = 3;
enum Alliance { kCrew = 0, kEnemy, kAllianceCount };
constexpr int kUnitBehaviorSimple = 1;
constexpr int kUnitBehaviorAttackWhenDiscovered = 2;
constexpr int kUnitBehaviorCrewMember = 3;
enum UnitBbEntry {
  kUnitTarget = 0,
  kUnitDestination = 1,
  kUnitAttackDestination = 2,
  kUnitBehavior = 3,
  kUnitAttacker = 4,
  kUnitTimer = 5,
  kUnitBbEntryMax = 6,
};

// Common Structures
struct Transform {
  v3f position;
  v3f scale = v3f(1.f, 1.f, 1.f);
};

const char* const deck_name[] = {
    "Cargo",
    "Bridge",
    "Observation",
};
constexpr uint64_t kMaxDeck = ARRAY_LENGTH(deck_name);
constexpr uint64_t kLastDeck = kMaxDeck - 1;

struct Asteroid {
  Transform transform;
  float mineral_source;
  float deplete;
};
DECLARE_GAME_TYPE(Asteroid, 8);

constexpr unsigned kNotifyAgeBits = 5;

constexpr int kMapDefaultBits = 6;
constexpr int kMapMaxWidth = 1 << kMapDefaultBits;
constexpr int kMapMaxHeight = 1 << kMapDefaultBits;
// Arrays of Tiles are everywhere: avoid specifying a constructor
// Initialization requires:
//   Tile t = kZeroTile;
// or
//   Tile tarray[128][128];
//   memset(tarray, 0, sizeof(tarray));
struct Tile {
  union {
    char position[6];
    struct {
      uint16_t cx;
      uint16_t cy;
      // Reference to the ship
      uint8_t ship_index;
      // number of bits used in cx and cy
      uint16_t bitrange_xy : 4;
      uint16_t ship_deck : 4;
    };
  };
  // Bitfields
  union {
    uint16_t flags;
    struct {
      // a wall, no movement
      uint16_t blocked : 1;
      // tile does not support human life
      uint16_t nooxygen : 1;
      // tile contents are 'unknown' unless marked visible
      uint16_t shroud : 1;
      // tile is visible to all players
      uint16_t visible : 1;
      // tile is exterior to the ship walls ('blocked' flag)
      uint16_t exterior : 1;
      // tile has ever been visible to a player
      uint16_t explored : 1;
    };
  };
};
struct Grid {
  Tile tilemap[kMapMaxHeight][kMapMaxWidth];
};
DECLARE_GAME_TYPE(Grid, 2);

enum ShipEnum { kShipBlank, kShipShuttle, kShipCruiser };

struct Ship {
  Transform transform;
  uint64_t level;
  uint64_t deck = 1;
  uint64_t pod_capacity;
  Tile* map;
  uint16_t map_width;
  uint16_t map_height;
  ShipEnum type;
  unsigned ftl_frame : 6;
  unsigned engine_animation : 4;
  unsigned PADDING : 22;
};
DECLARE_GAME_TYPE(Ship, 2);

struct Projectile {
  v3f start;
  v3f end;
  WeaponKind wkind;
  // Frame count since creation
  unsigned frame;
  // Frame duration
  int duration;
  uint32_t target_id;
};
DECLARE_GAME_TYPE(Projectile, 128);

#define COMMON_MEMBER_DECL \
  int id;                  \
  v3f position;            \
  v3f scale;               \
  v3f bounds;              \
  Tile tile;               \
  uint64_t control;        \
  uint64_t ship_index;     \
  uint64_t player_index;   \
  int type_id

enum EntityEnum {
  kEeInvalid = kInvalidId,
  kEeUnit,
  kEeModule,
};

struct Unit {
  COMMON_MEMBER_DECL = kEeUnit;
  Blackboard bb;
  // This will teleport unit position to the warp_tile at beginning of frame
  Tile warp_tile;
  UnitKind kind;
  Alliance alliance = kCrew;

  // Maybe need a weapon type?
  float attack_radius = 100.0f;
  int attack_frame;
  // Can attack every N frames.
  int attack_cooldown = 60;
  WeaponKind weapon_kind = kWeaponLaser;
  float health = 5.0f;
  float max_health = 5.0f;
  float speed = 1.f;
  UnitAction uaction;
  UnitAction persistent_uaction;
  uint32_t notify;
  bool dead;
  bool spacesuit;
  bool inspace;
};

constexpr int kTrainIdle = -1;

struct Module {
  COMMON_MEMBER_DECL = kEeModule;
  ModuleKind mkind;
  int frames_to_build = 200;
  int frames_building = 0;
  int frames_to_train = 1600;
  int frames_training = kTrainIdle;
  bool enabled = true;
};

union Entity {
  struct {
    COMMON_MEMBER_DECL;
  };
  Unit unit;
  Module module;

  Entity() : type_id(kEeInvalid)
  {
  }
};
#define MAX_ENTITY 128
DECLARE_GAME_TYPE_WITH_ID(Entity, MAX_ENTITY);
DECLARE_GAME_ENTITY(Unit, kEeUnit);
DECLARE_GAME_ENTITY(Module, kEeModule);

#define ZeroEntity(x)         \
  FreeHashEntryEntity(x->id); \
  *(Entity*)x = kZeroEntity;

struct Command {
  UnitAction type;
  v3f destination;
  uint32_t unit_id;
  unsigned control : MAX_PLAYER;
  uint64_t PADDING : 30;
};
DECLARE_GAME_QUEUE(Command, 16);

struct Player {
  Camera camera;  // Assumes fixed dimension window
  v3f selection_start;
  v3f mouse_world;
  Tile mouse_tile;
  ModuleKind mod_placement;
  uint64_t ship_index;
  bool admin_menu;
  v2f admin_menu_pos;
  bool tile_menu;
  v2f tile_menu_pos;
  v2f game_menu_pos;
  bool scenario_menu;
  bool mineral_cheat;
  float mineral = 0;
  uint64_t level = 1;
  unsigned hud_mode : kHudModeBits;
  unsigned local : 1;
  uint64_t PADDING : 58;
};
DECLARE_GAME_TYPE(Player, MAX_PLAYER);

constexpr int kMaxInvasionCount = 10;

// TODO: Find a better place for this. Controls the max units allowed in an
// invasion and increments up to kMaxInvasionCount.
static int kMaxThisInvasion = 2;

struct Invasion {
  Transform transform;
  v2f invasion_dir;
  // Units countained in the invasion.
  uint32_t unit_id[kMaxInvasionCount];
  int unit_count;

  bool docked = false;
  Tile docked_tile;
};
DECLARE_GAME_TYPE(Invasion, 2);
