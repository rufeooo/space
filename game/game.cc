#include "game.h"

#include <thread>

#include "event_buffer.h"

namespace game {

namespace {

struct State {
  // Run each game logic update with a delta_time of ms_per_update.
  std::chrono::milliseconds ms_per_update = 
      std::chrono::milliseconds(15);
  // Run a frame every min_ms_per_frame if the time it takes to run
  // update / render  is under this value the system will sleep until
  // it reaches min_ms_per_frame. This is really just to save battery
  // on my laptop at the moment. I'm not really sure it's worth always
  // doing.
  std::chrono::milliseconds min_ms_per_frame = 
      std::chrono::milliseconds(15);
  std::chrono::milliseconds game_time;
  std::chrono::milliseconds real_time;
  std::chrono::milliseconds ms_per_frame;
  bool paused = false;
  bool end = false;
  bool sleep_on_loop_end = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
};

static State kGameState;

// Game callbacks.
static Initialize _Initialize;
static ProcessInput _ProcessInput;
static HandleEvent _HandleEvent;
static Update _Update;
static Render _Render;
static OnEnd _OnEnd;

inline std::chrono::milliseconds NowMS() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}

}

void Setup(
    Initialize init_callback,
    ProcessInput input_callback,
    HandleEvent event_callback,
    Update update_callback,
    Render render_callback,
    OnEnd endcallback) {
  // Reset game state.
  kGameState = State();
  _Initialize = init_callback;
  _ProcessInput = input_callback;
  _HandleEvent = event_callback;
  _Update = update_callback;
  _Render = render_callback;
  _OnEnd = endcallback;
  // 2 kB event buffer.
  AllocateEventBuffer(2048);
}

// Runs the game.
bool Run(uint64_t loop_count) {
  if (!_Initialize()) {
    _OnEnd();
    return false;
  }
  auto previous = NowMS();
  std::chrono::milliseconds lag(0);
  kGameState.game_updates = 0;
  kGameState.game_time = std::chrono::milliseconds(0);
  std::chrono::milliseconds current, elapsed, endloop;
  while (loop_count == 0 || kGameState.game_updates < loop_count) {

    if (kGameState.end) {
      _OnEnd();
      return true;
    }

    current = NowMS();
    elapsed = current - previous;
    if (!kGameState.paused) lag += elapsed;

    _ProcessInput();
    kGameState.real_time += elapsed;

    while (!kGameState.paused &&
           lag >= kGameState.ms_per_update) {
      // Dequeue and handle all events in event queue.
      Event event;
      while (PollEvent(&event)) {
        _HandleEvent(event);
      }
      // Clears all memory in event buffer since they should
      // have all been handled by now.
      ResetEventBuffer();

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      _Update();

      lag -= kGameState.ms_per_update;
      kGameState.game_time += kGameState.ms_per_update;
      ++kGameState.game_updates;
    }

    if (!_Render()) {
      _OnEnd();
      return true; // Returns ??
    }

    endloop = NowMS();
    previous = current;
    auto ms = endloop - current;

    // sleep s.t. we only do min_ms_per_frame.
    if (kGameState.sleep_on_loop_end &&
        ms < kGameState.min_ms_per_frame) {
      auto sleep_time = kGameState.min_ms_per_frame - ms;
      std::this_thread::sleep_for(sleep_time);
    }

    kGameState.ms_per_frame = NowMS() - current;
  }

  _OnEnd();
  DeallocateEventBuffer();

  return true;
}

void Pause() {
  kGameState.paused = true;
}

void Resume() {
  kGameState.paused = false;
}

void End() {
  kGameState.end = true;
}

std::chrono::milliseconds Time() {
  return kGameState.game_time;
}

int Updates() {
  return kGameState.game_updates;
}

}
