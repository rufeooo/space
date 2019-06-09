#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "game.h"
#include "sdl_game.h"

TEST_CASE("Simple Game", "[game]") {
  class TestGame : public game::Game {
   public:
    void ProcessInput() override {}
    void Update() override {}
    void Render() override {}
  };
  uint64_t count = 5000;
  TestGame game;
  game.Run(count);
  REQUIRE(game.game_updates() == count);
  // Total game time should be game updates * microseconds_per_update.
  // 5000 * 10000 = 50000000
  REQUIRE(game.game_time() == std::chrono::microseconds(50000000));
}

TEST_CASE("Simple SDL Game", "[sdl-game]") {
  class TestSDLGame : public game::SDLGame {
   public:
    TestSDLGame() : game::SDLGame(500, 500) {}
    void Update() override {}
  };
  TestSDLGame game;
  game.Run(10000);
  // Simply verify a window and renderer were properly created and
  // initialized.
  REQUIRE(game.window() != nullptr);
  REQUIRE(game.renderer() != nullptr);
}
