#pragma once

#include <cstdint>

#include "event.h"

namespace game
{
// Function run on game init.
typedef bool (*Initialize)(void);

// Run at top of game loop to process input.
typedef bool (*ProcessInput)(void);

// Function called when event queue is dequeued.
typedef void (*HandleEvent)(Event event);

// Run N times for each game loop with fixed time step ms_per_update_.
typedef bool (*Update)(void);

// Run at end of game loop, typically used for rendering.
typedef bool (*Render)(void);

// Run once when game end is triggered.
typedef void (*OnEnd)(void);

// Setup game callbacks.
void Setup(Initialize init_callback, ProcessInput input_callback,
           HandleEvent event_callback, Update update_callback,
           Render render_callback, OnEnd end_callback);

// Runs the game.
bool Run(uint64_t loop_count = 0);

// End the game.
void End();

// Elapsed game milliseconds since start
uint64_t GameUsec();

// Game update since game start.
int Updates();

// Save events to a file.
void SaveEventsToFile();

// Replay events from a given file.
void LoadEventsFromFile(const char* filename);

}  // namespace game
