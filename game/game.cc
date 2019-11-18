#include "game.h"

#include <iostream>
#include <unordered_map>
#include <thread>

#include "event_buffer.h"

namespace game {

namespace {

struct State {
  // Run each game logic update with a delta_time of ms_per_update_.
  std::chrono::milliseconds ms_per_update_ = 
      std::chrono::milliseconds(15);
  // Run a frame every min_ms_per_frame if the time it takes to run
  // update / render  is under this value the system will sleep until
  // it reaches min_ms_per_frame_. This is really just to save battery
  // on my laptop at the moment. I'm not really sure it's worth always
  // doing.
  std::chrono::milliseconds min_ms_per_frame_ = 
      std::chrono::milliseconds(15);
  std::chrono::milliseconds game_time_;
  std::chrono::milliseconds real_time_;
  std::chrono::milliseconds ms_per_frame_;
  bool paused_ = false;
  bool end_ = false;
  bool sleep_on_loop_end_ = true;
  // Number of times the game has been updated.
  uint64_t game_updates_ = 0;
  uint64_t game_loops_ = 0;
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
    OnEnd end_callback) {
  // Reset game state.
  kGameState = State();
  _Initialize = init_callback;
  _ProcessInput = input_callback;
  _Update = update_callback;
  _Render = render_callback;
  _OnEnd = end_callback;
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
  kGameState.game_updates_ = 0;
  kGameState.game_time_ = std::chrono::milliseconds(0);
  std::chrono::milliseconds current, elapsed, end_loop;
  while (loop_count == 0 || kGameState.game_updates_ < loop_count) {

    if (kGameState.end_) {
      _OnEnd();
      return true;
    }

    current = NowMS();
    elapsed = current - previous;
    if (!kGameState.paused_) lag += elapsed;

    _ProcessInput();
    kGameState.real_time_ += elapsed;

    while (!kGameState.paused_ &&
           lag >= kGameState.ms_per_update_) {
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

      lag -= kGameState.ms_per_update_;
      kGameState.game_time_ += kGameState.ms_per_update_;
      ++kGameState.game_updates_;
    }

    if (!_Render()) {
      _OnEnd();
      return true; // Returns ??
    }

    end_loop = NowMS();
    previous = current;
    auto ms = end_loop - current;

    // sleep s.t. we only do min_ms_per_frame_.
    if (kGameState.sleep_on_loop_end_ &&
        ms < kGameState.min_ms_per_frame_) {
      auto sleep_time = kGameState.min_ms_per_frame_ - ms;
      std::this_thread::sleep_for(sleep_time);
    }

    kGameState.ms_per_frame_ = NowMS() - current;
  }
  _OnEnd();
  return true;
}

void Pause() {
  kGameState.paused_ = true;
}

void Resume() {
  kGameState.paused_ = false;
}

void End() {
  kGameState.end_ = true;
}

std::chrono::milliseconds Time() {
  return kGameState.game_time_;
}

int Updates() {
  return kGameState.game_updates_;
}

}
