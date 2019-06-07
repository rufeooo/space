#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>
#include <catch2/catch.hpp>

#include "game.h"

TEST_CASE("Simple Game", "[game]") {
  class TestGame : public game::Game {
   public:
    void RunSystems() override {}
    void RunRenderer() override {}
  };
  TestGame game;
  game.Run(5000000);
}
