#include "game.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "event_buffer.h"
#include "platform/filesystem.h"

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
  std::ofstream output_event_file;
  std::ifstream input_event_file;
  EventBuffer event_buffer;
};

static State kGameState;

// Game callbacks.
static Initialize _Initialize;
static ProcessInput _ProcessInput;
static HandleEvent _HandleEvent;
static Update _Update;
static Render _Render;
static OnEnd _OnEnd;

// 1 KiB should be plenty. It'd be unusual for the event
// queue to surpass 1KiB in a given frame.
constexpr int kEventBufferSize = 1024;

inline std::chrono::milliseconds NowMS() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}

void OptionallyPumpEventsFromFile() {
  auto& input = kGameState.input_event_file;
  if (!input.is_open()) return;
  // First 8 bytes of event file is the game loop.
  while (1) {
    if (input.eof()) return;
    auto cur_pos = input.tellg();
    if (cur_pos == -1) return;
    uint64_t update = 0;
    input.read((char*)&update, sizeof(uint64_t));
    if (update != kGameState.game_updates) {
      input.seekg(cur_pos);
      return;
    }
    // Seek forward in file past game loop.
    cur_pos += sizeof(uint64_t);
    input.seekg(cur_pos);
    // Get the size to copy the correct number of bytes into
    // the event buffer.
    uint16_t size = 0;
    input.read((char*)&size, sizeof(uint16_t));
    auto& event_buffer = kGameState.event_buffer;
    input.seekg(cur_pos);
    // Make sure the event buffer has memory allocated if it's
    // being used.
    assert(event_buffer.buffer != nullptr);
    input.read((char*)&event_buffer.buffer[event_buffer.idx],
               size + 4); // + 4 for the size and metadata
    event_buffer.idx += size + 4;
    cur_pos += size + 4;
    input.seekg(cur_pos);
  }
}

void OptionallyCloseEventsFile() {
  auto& input = kGameState.input_event_file;
  if (!input.is_open()) return;
  auto cur_pos = input.tellg();
  if (cur_pos != -1) return;
  input.close();
  SetCustomEventBuffer(nullptr);
}

void OptionallyWriteEventToFile(const Event& event) {
  auto& file = kGameState.output_event_file;
  if (!file.is_open()) return;
  // Write game loop first then event.
  file.write((char*)&kGameState.game_updates, sizeof(uint64_t));
  file.write((char*)&event.size, sizeof(uint16_t));
  file.write((char*)&event.metadata, sizeof(uint16_t));
  file.write((char*)&event.data[0], event.size);
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
  _HandleEvent = event_callback;
  _Update = update_callback;
  _Render = render_callback;
  _OnEnd = end_callback;
  // 2 kB event buffer.
  AllocateEventBuffer(kEventBufferSize);
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
      // Pump the event queue if a replay is coming from file.
      OptionallyPumpEventsFromFile();

      // Dequeue and handle all events in event queue.
      Event event;
      while (PollEvent(&event)) {
        OptionallyWriteEventToFile(event);
        _HandleEvent(event);
      }

      // Clears all memory in event buffer since they should
      // have all been handled by now.
      ResetEventBuffer();

      // If all events have been pumped from file close it
      // and reset the regular event buffer.
      OptionallyCloseEventsFile();

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

void SaveEventsToFile() {
  filesystem::MakeDirectory("_tmp");
  std::time_t time_now = std::time(nullptr);
  std::stringstream ss;
	ss << std::put_time(std::localtime(&time_now),
                      "_tmp/EVENTS_%y-%m-%d_%OH-%OM-%OS");
  kGameState.output_event_file.open(ss.str());
}

void LoadEventsFromFile(const char* filename) {
  // Setup the event buffer.
  auto& event_buffer = kGameState.event_buffer;
  if (!event_buffer.buffer) {
    event_buffer.buffer =
      (uint8_t*)calloc(kEventBufferSize, sizeof(uint8_t));
  }
  event_buffer.buffer_size = kEventBufferSize;
  event_buffer.idx = 0;
  event_buffer.poll_idx = 0;
  // Now load the file.
  kGameState.input_event_file.open(filename);
  SetCustomEventBuffer(&event_buffer);
}

}
