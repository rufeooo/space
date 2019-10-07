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
  // Gives the user a chance to run any necessary 'game end' code.
  virtual void OnGameEnd() {}

  // Setting to 0 runs indefinitely OR Render return false OR 'end_'
  // is triggered via the End() call.
  bool Run(uint64_t loop_count=0);
  void Pause();
  void Resume();
  void End();

  uint64_t game_updates() const { return game_updates_; }
  std::chrono::milliseconds game_time() const { return game_time_; }
  std::chrono::milliseconds ms_per_frame() const {
    return ms_per_frame_;
  }
  int ms_per_update() const {
    return ms_per_update_.count();
  }
  float fps() const { return (1.f / ms_per_frame_.count()) * 1000.f; }
 private:
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

}
