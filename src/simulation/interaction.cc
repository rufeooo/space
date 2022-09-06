
#include "camera.cc"
#include "simulation.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"

namespace simulation
{
static uint64_t kInputHash = DJB2_CONST;
static uint64_t kDebugInputHash;
static uint64_t kDebugSimulationHash;
static float kCameraSpeed = 4.f;

void
CacheSyncHashes(bool update, uint64_t frame)
{
  kDebugInputHash ^= (update * simulation::kInputHash);
  kDebugSimulationHash ^= (update * simulation::kSimulationHash);
#ifdef DEBUG_SYNC
  printf(
      "[Frame %u] [DebugInputHash 0x%016lx] [DebugSimulationHash 0x016%lx]\n",
      frame, kDebugInputHash, kDebugSimulationHash);
#endif
}

void
DebugPanel(const Player& player, uint32_t tag, const Stats& stats,
           uint64_t frame_target_usec, uint64_t frame, uint64_t jerk)
{
  auto sz = player.camera.viewport;
#define BUFFER_SIZE 64
  char buffer[BUFFER_SIZE];
  static bool enable_debug = false;
  imui::PaneOptions options;
  imui::Begin(v2f(3.f, sz.y - 30.f), tag, options);
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
    snprintf(buffer, BUFFER_SIZE,
             "Network Rtt: [%06lu us to %06lu us] [%lu/%lu queue]",
             kNetworkState.egress_min * frame_target_usec,
             kNetworkState.egress_max * frame_target_usec,
             NetworkContiguousSlotReady(frame), MAX_NETQUEUE);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Window Size: %ix%i", (int)sz.x, (int)sz.y);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Mouse Pos In World: (%.1f,%.1f,%.1f)",
             player.world_mouse.x, player.world_mouse.y, player.world_mouse.z);
    imui::Text(buffer);
    v2i mouse_tile;
    if (WorldToTilePos(player.world_mouse, &mouse_tile)) {
      snprintf(buffer, BUFFER_SIZE, "Mouse Pos To Tile: (%i,%i)",
               mouse_tile.x, mouse_tile.y);
      imui::Text(buffer);
    }
    snprintf(buffer, BUFFER_SIZE, "Input hash: 0x%lx", kDebugInputHash);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Sim hash: 0x%lx", kDebugSimulationHash);
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
    ScenarioReset(false);                            \
  }
      imui::Indent(2);
      UI_TOGGLE_FEATURE(ship);
      UI_TOGGLE_FEATURE(asteroid);
      UI_TOGGLE_FEATURE(missile);
      UI_TOGGLE_FEATURE(pod);
      UI_TOGGLE_FEATURE(invasion);
      UI_TOGGLE_FEATURE(mine);
      imui::Indent(-2);
    }
    if (imui::Text("Exit", debug_options).clicked) {
      exit(1);
    }
    imui::Indent(-2);
  }

  snprintf(buffer, BUFFER_SIZE, "Minerals: %.1f", player.resource.mineral);
  imui::Text(buffer);
  snprintf(buffer, BUFFER_SIZE, "Level: %lu", player.level);
  imui::Text(buffer);

  if (simulation::kSimulationOver) {
    snprintf(buffer, BUFFER_SIZE, "Game Over");
    imui::Text(buffer);
  }

  imui::End();
}

void
LogPanel(v2f screen_dims, uint32_t tag)
{
  const float width = 300.0f;
  const float height = 100.0f;
  imui::PaneOptions pane_options(width, height);
  imui::TextOptions text_options;
  text_options.scale = 0.7f;
  imui::Begin(v2f(screen_dims.x - width, 0.0f), tag, pane_options);
  for (int i = 0, imax = LogCount(); i < imax; ++i) {
    const char* log_msg = ReadLog(i);
    if (!log_msg) continue;
    imui::Text(log_msg, text_options);
  }
  imui::End();
}

void
RenderBlackboard(const Unit* unit)
{
  char txt[64];
  for (int i = 0; i < kUnitBbEntryMax; ++i) {
    UnitBbEntry bb_entry = (UnitBbEntry)i;
    switch (bb_entry) {
      case kUnitDestination: {
        const v3f* d = nullptr;
        if (!BB_GET(unit->bb, kUnitDestination, d)) continue;
        snprintf(txt, 64, "dest: %.0f,%.0f", d->x, d->y);
        imui::Text(txt);
      } break;
      case kUnitAttackDestination: {
        const v3f* d = nullptr;
        if (!BB_GET(unit->bb, kUnitAttackDestination, d)) continue;
        snprintf(txt, 64, "adest: %.0f,%.0f", d->x, d->y);
        imui::Text(txt);
      } break;
      case kUnitTarget: {
        const int* t = nullptr;
        if (!BB_GET(unit->bb, kUnitTarget, t)) continue;
        snprintf(txt, 64, "target: %i", *t);
        imui::Text(txt);
      } break;
      case kUnitBehavior: {
        const int* b = nullptr;
        if (!BB_GET(unit->bb, kUnitBehavior, b)) continue;
        snprintf(txt, 64, "behavior: %i", *b);
        imui::Text(txt);
      } break;
      case kUnitAttacker: {
        const int* t = nullptr;
        if (!BB_GET(unit->bb, kUnitAttacker, t)) continue;
        snprintf(txt, 64, "attacker: %i", *t);
        imui::Text(txt);
      } break;
      default: {
        snprintf(txt, 64, "set: %i", i);
        if (BB_EXI(unit->bb, i)) {
          imui::Text(txt);
        }
      }
    }
  }
}

void
HudSelection(v2f screen)
{
  // selected Unit text
  uint64_t player_control = (1 << kPlayerIndex);
  const Unit* unit = nullptr;
  for (int i = 0; i < kUsedUnit; ++i) {
    if (0 == (kUnit[i].control & player_control)) continue;
    unit = &kUnit[i];
    break;
  }

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

  for (int i = 0; i < MAX_SELECTED_TEXT; ++i) {
    imui::Text(selected_text[i]);
  }
  imui::Text("Blackboard");
  RenderBlackboard(unit);
}

void
DebugHudAI(v2f screen)
{
  char txt[64];
  static bool render_all_ai_data = false;
  int unit_ai_count = 0;
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    const int* behavior;
    if (!BB_GET(unit->bb, kUnitBehavior, behavior)) continue;
    snprintf(txt, 64, "AI Unit %i", unit->id);
    if (imui::Text(txt).highlighted || render_all_ai_data) {
      imui::Indent(1);
      RenderBlackboard(unit);
      imui::Indent(-1);
    }
    unit_ai_count++;
  }
  if (!unit_ai_count) return;
  imui::TextOptions options;
  options.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text("Render All", options).clicked) {
    render_all_ai_data = !render_all_ai_data;
  }
}

void
Hud(v2f screen, uint32_t tag)
{
  imui::Begin(v2f(screen.x - 225.f, screen.y - 30.0f), tag);
  HudSelection(screen);
  // TODO(abrunasso): Perhaps enable / disable this in Debug panel?
  DebugHudAI(screen);
  imui::End();
}

void
ControlEvent(const PlatformEvent event, uint64_t player_index, Player* player)
{
  v3f world_pos = camera::ScreenToWorldSpace(&player->camera, event.position);
  imui::MousePosition(event.position, player_index);
  djb2_hash_more((const uint8_t*)&event, sizeof(PlatformEvent), &kInputHash);
  switch (event.type) {
    case MOUSE_POSITION: {
      player->world_mouse = world_pos;
    } break;
    case MOUSE_WHEEL: {
      // TODO(abrunasso): Why does this need to be negative?
      player->camera.motion.z = -10.f * event.wheel_delta;
    } break;
    case MOUSE_DOWN: {
      imui::MouseClick(event.position, event.button, player_index);

      if (event.button == BUTTON_MIDDLE) {
        camera::Move(&player->camera, world_pos);
      }

      if (event.button == BUTTON_LEFT) {
        switch (player->hud_mode) {
          case kHudSelection: {
            player->selection_start = world_pos;
            UnselectPlayerAll(player_index);
          } break;
          case kHudAttackMove: {
            LOGFMT("Order attack move [%.0f,%.0f]", world_pos.x, world_pos.y);
            PushCommand({kUaAttackMove, world_pos, kInvalidUnit,
                         (unsigned)(1 << player_index)});
          } break;
          case kHudModule: {
            ModuleKind mkind = player->mod_placement;
            uint64_t grid = TilemapWorldToGrid(world_pos);
            if (grid == kInvalidIndex) break;
            TilemapSet(grid);
            v2i tilepos;
            if (WorldToTilePos(world_pos, &tilepos) &&
                event.button == BUTTON_LEFT) {
              if (!ModuleCanBuild(mkind, player)) {
                LOGFMT("Player can't afford module %i", mkind);
                break;
              }
              Module* m = UseModule();
              m->ship_index = grid;
              m->tile = tilepos;
              m->mkind = mkind;
              m->bounds = ModuleBounds(mkind);
              m->player_id = player_index;
              player->resource.mineral -= ModuleCost(mkind);
            }
            LOGFMT("Order build [%i] [%i,%i]", mkind, tilepos.x, tilepos.y);
            PushCommand({kUaBuild, world_pos, kInvalidUnit,
                         (unsigned)(1 << player_index)});
          } break;
        }
      } else if (event.button == BUTTON_RIGHT) {
        Unit* target = GetUnitTarget(player_index, world_pos);
        if (target) {
          LOGFMT("Order attack [%lu]", target->id);
          PushCommand({kUaAttack, world_pos, kInvalidUnit,
                       (unsigned)(1 << player_index)});
        } else {
          LOGFMT("Order move [%.0f,%.0f]", world_pos.x, world_pos.y);
          PushCommand({kUaMove, world_pos, kInvalidUnit,
                       (unsigned)(1 << player_index)});
        }
      }

      player->hud_mode = kHudDefault;
    } break;
    case MOUSE_UP: {
      if (event.button == BUTTON_LEFT) {
        // TODO(abrunasso): Unconvinced this is the best way to check if a
        // selection occurred. Also unconvined that it's not....
        Unit* unit = nullptr;
        if (player->selection_start.x != 0.f ||
            player->selection_start.y != 0.f ||
            player->selection_start.z != 0.f) {
          v3f diff = player->world_mouse - player->selection_start;
          math::Rectf sbox(player->selection_start.x, player->selection_start.y,
                           diff.x, diff.y);
          sbox = math::OrientToAabb(sbox);
          bool selected = false;
          for (int i = 0; i < kUsedUnit; ++i) {
            unit = GetUnit(sbox, i);
            if (!unit) continue;
            LOGFMT("Select unit: %i", unit->id);
            SelectPlayerUnit(player_index, unit);
            selected = true;
          }

          // Prefer selecting units over modules.
          for (int i = 0; i < kUsedModule && !selected; ++i) {
            Module* mod = GetModule(sbox, i);
            if (!mod) continue;
            LOGFMT("Select Module: %i", i);
            SelectPlayerModule(player_index, mod);
            selected = true;
          }
        }
        player->selection_start = v3f(0.f, 0.f, 0.f);
        // Box selection missed, fallback to single unit selection
        if (!unit) {
          unit = GetUnit(world_pos);
          SelectPlayerUnit(player_index, unit);
        }
      }
    } break;
    case KEY_DOWN: {
      switch (event.key) {
        case 27 /* ESC */: {
          exit(1);
        } break;
        case 'w': {
          player->camera.motion.y = kCameraSpeed;
        } break;
        case 'a': {
          player->camera.motion.x = -kCameraSpeed;
        } break;
        case 's': {
          player->camera.motion.y = -kCameraSpeed;
        } break;
        case 'd': {
          player->camera.motion.x = kCameraSpeed;
        } break;
        case 'r': {
          // This check has to happen, otherwise the cursor will go into attack
          // move with no units selected and you won't be able to select
          // units or attack without left clicking again.
          if (CountUnitSelection(player_index) > 0) {
            player->hud_mode = kHudAttackMove;
          }
        } break;
        case ' ': {
          for (int i = 0; i < kUsedShip; ++i) {
            if (kShip[i].level != player->level) continue;
            bool ftl_ready = kShip[i].sys[kModEngine] > .5f;
            if (!ftl_ready) {
              LOG("Faster Than Light engine is offline!");
            } else if (player->resource.mineral >= kFtlCost) {
              LOG("Faster Than Light engine activated!");
              kShip[i].ftl_frame = 1;
              player->resource.mineral -= kFtlCost;
            } else {
              LOGFMT("Ftl requires minerals [%d]!", kFtlCost);
            }
          }
        } break;
        //case '1': {
        //  player->hud_mode = kHudModule;
        //  player->mod_placement = 0;
        //} break;
        //case '2': {
        //  player->hud_mode = kHudModule;
        //  player->mod_placement = 1;
        //} break;
        case '1': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModMine;
        } break;
        case '2': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModBarrack;
        } break;
        case '3': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModMedbay;
        } break;
        case 'h': {
          player->level = CLAMP(player->level - 1, 1, 10);
        } break;
        case 'l': {
          player->level = CLAMP(player->level + 1, 1, 10);
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
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
GameUI(v2f screen, uint32_t tag, int player_index, Player* player)
{
  char buffer[BUFFER_SIZE];
  imui::Begin(v2f(0.f, 100.f), tag);
  imui::Result hud_result;
  v2f p(50.f, 10.f);
  for (int i = 3; i < kModCount; ++i) {
    v3f c = ModuleColor((ModuleKind)i);
    hud_result = imui::Button(math::Rectf(p.x, p.y, 50, 50),
                              v4f(c.x, c.y, c.z, .6f));
    p.x += 55.f;
    if (hud_result.clicked) {
      player->hud_mode = kHudModule;
      player->mod_placement = (ModuleKind)i;
    }
  }
  for (int i = 0; i < kUsedModule; ++i) {
    Module* m = &kModule[i];
    unsigned player_control = (1 << player_index);
    if (0 == (m->control & player_control)) continue;
    snprintf(buffer, BUFFER_SIZE, "%s %s", ModuleName(m->mkind),
             m->enabled ? "Enabled" : "Disabled");
    // TODO: This doesn't work because the imui state for it dissapears the
    // the moment left click is pressed. The fix for this may be interesting.
    // It is likely worth having some idea of imui bounds on the screen and
    // to check against those bounds before letting the click work in world
    // space.
    if (imui::Text(buffer).clicked) {
      m->enabled = !m->enabled;
    }
  }
  imui::End();
}

void
ProcessSimulation(int player_index, uint64_t event_count,
                  const PlatformEvent* event)
{
  Player* p = &kPlayer[player_index];
  for (int i = 0; i < event_count; ++i) {
    ControlEvent(event[i], player_index, p);
  }
}

}  // namespace simulation
