#include "sdl_game.h"

namespace game {

void SDLGame::Initialize() {
  int flags = SDL_WINDOW_SHOWN;
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    return;
  }
  if (SDL_CreateWindowAndRenderer(
      window_width_, window_height_, flags, &window_, &renderer_)) {
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
  SDL_RenderPresent(renderer_);
}

}
