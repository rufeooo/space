#pragma once

#include <chrono> 
#include <cstdint>

namespace game {

class Game {
 public:
  virtual ~Game() = default;
  // Returns true if game successfully initialized.
  virtual bool Initialize() { return true; }
  // Returns true if input was processed sucessfully.
  virtual bool ProcessInput() = 0;
  // Returns true if the update logic ran sucessfully.
  virtual bool Update() = 0;
  // Returns true if the game should continue to render another frame.
  // this gives the render engine a chance to terminate the game if
  // the user does something like close the window.
  virtual bool Render() = 0;

  // Setting to 0 runs indefinitely.
  bool Run(uint64_t loop_count=0);
  void Pause();
  void Resume();
  void End();

  uint64_t game_updates() const { return game_updates_; }
  std::chrono::milliseconds game_time() const { return game_time_; }
  std::chrono::milliseconds ms_per_frame() const {
    return ms_per_frame_;
  }
  std::chrono::milliseconds ms_per_update() const {
    return ms_per_update_;
  }
  float fps() const { return (1.f / ms_per_frame_.count()) * 1000.f; }
 private:
  // Roughly 1/60th of a second the game should update.
  std::chrono::milliseconds ms_per_update_ = 
      std::chrono::milliseconds(15);
  // Only run a game loop once per 5 milliseconds.
  std::chrono::milliseconds min_ms_per_frame_ = 
      std::chrono::milliseconds(15);
  std::chrono::milliseconds game_time_;
  std::chrono::milliseconds real_time_;
  std::chrono::milliseconds ms_per_frame_;
  int fps_;
  bool paused_ = false;
  bool end_ = false;
  bool sleep_on_loop_end_ = true;
  // Number of times the game has been updated.
  uint64_t game_updates_ = 0;
  uint64_t game_loops_ = 0;
};

}
