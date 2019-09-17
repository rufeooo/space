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
  std::chrono::microseconds game_time() const { return game_time_; }
 private:
  std::chrono::microseconds microseconds_per_update_ = 
      std::chrono::microseconds(10000);
  std::chrono::microseconds game_time_;
  std::chrono::microseconds real_time_;
  bool paused_ = false;
  bool end_ = false;
  // Number of times the game has been updated.
  uint64_t game_updates_;
};

}
