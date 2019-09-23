#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "game.h"
#include "sdl_game.h"

TEST_CASE("Simple Game", "[game]") {
  class TestGame : public game::Game {
   public:
    bool ProcessInput() override { return true; }
    bool Update() override { return true; }
    bool Render() override { return true; }
  };
  uint64_t count = 500;
  TestGame game;
  game.Run(count);
  REQUIRE(game.game_updates() == count);
  // Total game time should be game updates * milliseconds_per_update.
  // 500 * 15 = 7500
  REQUIRE(game.game_time() == std::chrono::milliseconds(7500));
}

// TODO: Remove SDL.
//TEST_CASE("Simple SDL Game", "[sdl-game]") {
  //class TestSDLGame : public game::SDLGame {
  // public:
  //  TestSDLGame() : game::SDLGame(500, 500) {}
  //  bool Update() override { return false; }
  //};
  //TestSDLGame game;
  //game.Run(10000);
  // Simply verify a window and renderer were properly created and
  // initialized. You should see a black screen pop up for this test.
  //REQUIRE(game.window() != nullptr);
  //REQUIRE(game.renderer() != nullptr);
//}
