#pragma once

#include "game.h"
#include "SDL.h"

namespace game {

class SDLGame : public Game {
 public:
  SDLGame(int width, int height) :
    window_width_(width), window_height_(height) {}

  ~SDLGame();

  void Initialize() override;
  void ProcessInput() override;
  void Render() override;

  SDL_Window* window() { return window_; }
  SDL_Renderer* renderer() { return renderer_; }

 private:
  int window_width_;
  int window_height_;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  SDL_Event event_;
};

}
