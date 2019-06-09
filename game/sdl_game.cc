#include "sdl_game.h"

#include <cassert>

namespace game {

SDLGame::~SDLGame() {
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
  }
  if (window_) {
    SDL_DestroyWindow(window_);
  }
  SDL_Quit();
}

void SDLGame::Initialize() {
  int flags = SDL_WINDOW_SHOWN;
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    // TODO: Implement some sort of logging here and assert.
    assert(0);
    return;
  }
  if (SDL_CreateWindowAndRenderer(
      window_width_, window_height_, flags, &window_, &renderer_)) {
    // TODO: Implement some sort of logging here and assert.
    assert(0);
    return;
  }
}

void SDLGame::ProcessInput() {
  if (SDL_PollEvent(&event_)) {
    switch(event_.type) {
      case SDL_QUIT: {
        End();
        break;
      }
      default: break;
    }
  }
}

void SDLGame::Render() {
  // Clear the screen to white.
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);
  SDL_RenderPresent(renderer_);
}

}
