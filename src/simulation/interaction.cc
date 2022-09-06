
#include "camera.cc"
#include "simulation.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"

namespace simulation
{
enum LeftClickMode {
  kSelect,
  kAttackMove,
};

LeftClickMode kLeftClickMode = kSelect;

static uint64_t kInputHash = DJB2_CONST;

bool
ShipFtlReady(uint64_t ship_index)
{
  return kShip[ship_index].sys[kModEngine] > .5f;
}

void
ControlShipFtl(uint64_t ship_index)
{
}

bool
ShouldAttack(uint64_t unit, uint64_t target)
{
  printf("Should %u attack %u\n", unit, target);
  if (unit == kInvalidUnit || target == kInvalidUnit) return false;
  Unit* controlled_unit = FindUnit(unit);
  Unit* target_unit = FindUnit(target);
  if (target_unit->alliance == controlled_unit->alliance) return false;
  printf("Should %u attack %u YES\n", unit, target);
  return true;
}

uint32_t
UnitId()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    if (!kUnit[i].kind) return kUnit[i].id;
  }
  return kInvalidUnit;
}

void
DebugPanel(const Player& player, const Stats& stats, uint64_t frame_target_usec)
{
  auto sz = window::GetWindowSize();
#define BUFFER_SIZE 64
  char buffer[BUFFER_SIZE];
  static bool enable_debug = false;
  imui::PaneOptions options;
  imui::Begin(v2f(3.f, sz.y - 30.f), options);
  imui::TextOptions debug_options;
  debug_options.color = gfx::kWhite;
  debug_options.highlight_color = gfx::kRed;
  if (imui::Text("Debug Menu", debug_options).clicked) {
    enable_debug = !enable_debug;
  }

  if (enable_debug) {
    snprintf(buffer, BUFFER_SIZE, "Frame Time: %04.02f us [%02.02f%%]",
             StatsMean(&stats), 100.f * StatsUnbiasedRsDev(&stats));
    imui::Indent(2);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Network Rtt: %06lu us [%lu/%lu queue]",
             kNetworkState.last_egress * frame_target_usec, NetworkReadyCount(),
             MAX_NETQUEUE);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Window Size: %ix%i", (int)sz.x, (int)sz.y);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Mouse Pos In World: (%.1f,%.1f)",
             player.world_mouse.x, player.world_mouse.y);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Input hash: 0x%lx", kInputHash);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Sim hash: 0x%lx", kSimulationHash);
    imui::Text(buffer);
    const char* ui_err = imui::LastErrorString();
    if (ui_err) imui::Text(ui_err);
    imui::Indent(-2);
  }

  static bool enable_game_menu = false;
  if (imui::Text("Game Menu", debug_options).clicked) {
    enable_game_menu = !enable_game_menu;
  }

  if (enable_game_menu) {
    imui::Indent(2);
    if (imui::Text("Reset Game", debug_options).clicked) {
      Reset(kNetworkState.game_id);
    }
    static bool scenario_menu = false;
    if (imui::Text("Scenario", debug_options).clicked) {
      scenario_menu = !scenario_menu;
    }
    if (scenario_menu) {
      imui::Indent(2);
      for (int i = 0; i < Scenario::kMaxScenario; ++i) {
        if (imui::Text(kScenarioNames[i], debug_options).clicked) {
          kScenario.type = (Scenario::Type)i;
          Reset(kNetworkState.game_id);
        }
      }
      imui::Indent(-2);
    }
    static bool features_menu = false;
    if (imui::Text("Features", debug_options).clicked) {
      features_menu = !features_menu;
    }
    if (features_menu) {
#define UI_TOGGLE_FEATURE(name)                      \
  snprintf(buffer, BUFFER_SIZE, "%s: %s", #name,     \
           kScenario.name ? "enabled" : "disabled"); \
  if (imui::Text(buffer, debug_options).clicked) {   \
    kScenario.name = !kScenario.name;                \
    ResetScenario(false);                            \
  }
      imui::Indent(2);
      UI_TOGGLE_FEATURE(ai);
      UI_TOGGLE_FEATURE(ship);
      UI_TOGGLE_FEATURE(asteroid);
      UI_TOGGLE_FEATURE(missile);
      UI_TOGGLE_FEATURE(pod);
      UI_TOGGLE_FEATURE(tilemap);
      imui::Indent(-2);
    }
    imui::Indent(-2);
  }

  snprintf(buffer, BUFFER_SIZE, "Minerals: %lu", kResource[0].mineral);
  imui::Text(buffer);
  snprintf(buffer, BUFFER_SIZE, "Level: %lu", player.level);
  imui::Text(buffer);

  if (simulation::SimulationOver()) {
    snprintf(buffer, BUFFER_SIZE, "Game Over");
    imui::Text(buffer);
  }
  v4f ready_color = v4f(1.0f, 0.0f, 1.0f, 0.75f);
  v4f unready_color = v4f(0.3f, 0.3f, 0.3f, 0.5f);
  for (int i = 0; i < kUsedShip; ++i) {
    bool ftl_ready = simulation::ShipFtlReady(i);
    if (imui::Button(math::Rectf(50, 100 + (i * 50), 40, 40),
                     ftl_ready ? ready_color : unready_color)
            .clicked) {
      if (!ftl_ready) {
        LOG("Faster Than Light engine is offline!");
      } else if (kResource[0].mineral >= kFtlCost) {
        LOG("Faster Than Light engine activated!");
        kShip[i].ftl_frame = 1;
        kResource[0].mineral -= kFtlCost;
      } else {
        LOGFMT("Ftl requires minerals [%d]!", kFtlCost);
      }
    }
  }

  {
    v4f orange(1.0f, .3f, .3f, .75f);
    int level_change = 0;
    if (imui::Button(math::Rectf(10, 125, 30, 30), orange).clicked) {
      level_change = -1;
    }
    if (imui::Button(math::Rectf(100, 125, 30, 30), orange).clicked) {
      level_change = 1;
    }

    {
      Player* p = &kPlayer[&player - kPlayer];
      p->level = CLAMP(p->level + level_change, 1, 10);
    }
  }

  imui::End();
}

void
LogPanel()
{
  imui::PaneOptions pane_options(300.0f, 100.0f);
  imui::TextOptions text_options;
  text_options.scale = 0.7f;
  imui::Begin(v2f(0.f, 0.f), pane_options);
  for (int i = 0, imax = LogCount(); i < imax; ++i) {
    const char* log_msg = ReadLog(i);
    if (!log_msg) continue;
    imui::Text(log_msg, text_options);
  }
  imui::End();
}

void
Hud(v2f screen)
{
  v2f dims(40, 40);
  if (imui::Button(math::Rectf(screen.x - 10 - dims.x, 100, dims.x, dims.y),
                   v4f(1.0f, 0.0f, 1.0f, 0.75f))
          .clicked) {
    kPlayer[kNetworkState.player_id].mod_placement = 1;
    LOG("Module placement.");
  }

  // selected Unit text
  uint64_t selected = UINT64_MAX;
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (IsUnitSelected(unit->id)) continue;
    selected = unit->id;
    break;
  }

  Unit* unit = FindUnit(selected);
  if (!unit) return;
  constexpr int MAX_SELECTED_TEXT = CREWA_MAX + 3;
  char selected_text[MAX_SELECTED_TEXT][64];
  {
    int t = 0;
    for (; t < CREWA_MAX; ++t) {
      snprintf(selected_text[t], 64, "[%u,%u] %s", unit->aknown_min[t],
               unit->aknown_max[t], crew_aname[t]);
    }
    snprintf(selected_text[t++], 64, "(%04.02f,%04.02f)",
             unit->transform.position.x, unit->transform.position.y);
    snprintf(selected_text[t++], 64, "uaction: %d", unit->uaction);
    snprintf(selected_text[t++], 64, "id: %i", unit->id);
  }

  imui::Begin(v2f(screen.x - 225.f, screen.y - 30.0f));
  for (int i = 0; i < MAX_SELECTED_TEXT; ++i) {
    imui::Text(selected_text[i]);
  }
  imui::Text("Blackboard");
  for (int i = 0; i < kUnitBbEntryMax; ++i) {
    UnitBbEntry bb_entry = (UnitBbEntry)i;
    switch (bb_entry) {
      case kUnitDestination: {
        v3f* d = nullptr;
        if (!BB_GET(unit->bb, kUnitDestination, d)) continue;
        snprintf(selected_text[0], 64, "dest: %.0f,%.0f", d->x, d->y);
        imui::Text(selected_text[0]);
      } break;
      case kUnitAttackDestination: {
        v3f* d = nullptr;
        if (!BB_GET(unit->bb, kUnitAttackDestination, d)) continue;
        snprintf(selected_text[0], 64, "attack dest: %.0f,%.0f", d->x, d->y);
        imui::Text(selected_text[0]);
      } break;
      case kUnitTarget: {
        int* t = nullptr;
        if (!BB_GET(unit->bb, kUnitTarget, t)) continue;
        snprintf(selected_text[0], 64, "target: %i", *t);
        imui::Text(selected_text[0]);
      } break;
      default: {
        snprintf(selected_text[0], 64, "set: %i", i);
        if (BB_EXI(unit->bb, i)) {
          imui::Text(selected_text[0]);
        }
      }
    }
  }
  imui::End();
}

void
ControlEvent(const PlatformEvent* event, Player* player)
{
  v3f world_pos = camera::ScreenToWorldSpace(
      &player->camera, v3f(event->position - window::GetWindowSize() * 0.5f));

  djb2_hash_more((const uint8_t*)event, sizeof(PlatformEvent), &kInputHash);
  switch (event->type) {
    case MOUSE_POSITION: {
      player->world_mouse = world_pos.xy();
      player->world_selection.width = world_pos.x - player->world_selection.x;
      player->world_selection.height = world_pos.y - player->world_selection.y;
    } break;
    case MOUSE_WHEEL: {
      // TODO(abrunasso): Why does this need to be negative?
      player->camera.motion.z = -0.1f * event->wheel_delta;
    } break;
    case MOUSE_DOWN: {
      imui::MouseClick(event->position, event->button);

      if (kPlayer[kNetworkState.player_id].mod_placement) {
        unsigned mkind = kPlayer[kNetworkState.player_id].mod_placement;
        kPlayer[kNetworkState.player_id].mod_placement = 0;
        if (event->button == BUTTON_LEFT) {
          for (int i = 0; i < kUsedShip; ++i) {
            v2i tilepos = WorldToTilePos(world_pos);
            if (!TileOk(tilepos)) continue;
            Module* m = UseModule();
            m->ship_index = i;
            m->cx = tilepos.x;
            m->cy = tilepos.y;
            m->mkind = mkind;
          }
        }
        break;
      }

      if (event->button == BUTTON_LEFT) {
        switch (kLeftClickMode) {
          case kSelect: {
            player->world_selection.x = world_pos.x;
            player->world_selection.y = world_pos.y;
            UnselectAll();
          } break;
          case kAttackMove: {
            LOGFMT("Order attack move [%.0f,%.0f]", world_pos.x, world_pos.y);
            PushCommand({kUaAttackMove, world_pos, kInvalidUnit});
          } break;
        }
        kLeftClickMode = kSelect;
        break;
      } else if (event->button == BUTTON_RIGHT) {
        uint32_t target = GetUnitId(world_pos);
        if (target != kInvalidUnit) {
          LOGFMT("Order attack [%lu]", target);
          PushCommand({kUaAttack, world_pos, kInvalidUnit});
        } else {
          LOGFMT("Order move [%.0f,%.0f]", world_pos.x, world_pos.y);
          PushCommand({kUaMove, world_pos, kInvalidUnit});
        }
      }
    } break;
    case MOUSE_UP: {
      if (event->button == BUTTON_LEFT) {
        // TODO(abrunasso): Unconvinced this is the best way to check if a
        // selection occurred. Also unconvined that it's not....
        uint32_t id = kInvalidUnit;
        if (player->world_selection.x != 0.f || 
            player->world_selection.y != 0.f) {
          math::Rectf aabb_selection = math::OrientToAabb(player->world_selection);
          for (int i = 0; i < kUsedUnit; ++i) {
            if (!GetUnitId(aabb_selection, i, &id)) continue;
            LOGFMT("Select unit: %i", id);
            SelectUnit(id);
          }
        }
        player->world_selection.x = 0.f;
        player->world_selection.y = 0.f;

        // Box selection missed, fallback to single unit selection
        if (id == kInvalidUnit) {
          id = GetUnitId(world_pos);
          SelectUnit(id);
        }
      }
    } break;
    case KEY_DOWN: {
      switch (event->key) {
        case 'w': {
          player->camera.motion.y = 1.f;
        } break;
        case 'a': {
          player->camera.motion.x = -1.f;
        } break;
        case 's': {
          player->camera.motion.y = -1.f;
        } break;
        case 'd': {
          player->camera.motion.x = 1.f;
        } break;
        case 'r': {
          if (kUsedSelection > 0) {
            kLeftClickMode = kAttackMove;
          }
        } break;
        case 'u': {
          v3f pos = camera::ScreenToWorldSpace(
              &player->camera,
              v3f(event->position - window::GetWindowSize() * 0.5f));
          float dsq;
          uint64_t unit_index =
              v3fNearTransform(pos, GAME_ITER(Unit, transform), &dsq);
          if (unit_index < kUsedUnit) {
            Notify* n = UseNotify();
            n->position = kUnit[unit_index].transform.position;
            n->age = 1;
          }
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event->key) {
        case 'w': {
          player->camera.motion.y = 0.f;
        } break;
        case 'a': {
          player->camera.motion.x = 0.f;
        } break;
        case 's': {
          player->camera.motion.y = 0.f;
        } break;
        case 'd': {
          player->camera.motion.x = 0.f;
        } break;
        default:
          break;
      }
    } break;
    default:
      break;
  }
}

void
ProcessSimulation(int player_id, uint64_t event_count,
                  const PlatformEvent* event)
{
  // Shared player control of the ship for now
  for (int i = 0; i < event_count; ++i) {
    Player* p = &kPlayer[player_id];
    ControlEvent(&event[i], p);
  }
}

}  // namespace simulation
