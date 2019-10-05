#include <iostream>

#include "gtest/gtest.h"
#include "game.h"

TEST(Game, HappyPathGameTest) {
  class TestGame : public game::Game {
   public:
    bool ProcessInput() override { return true; }
    bool Update() override { return true; }
    bool Render() override { return true; }
  };
  uint64_t count = 500;
  TestGame game;
  game.Run(count);
  ASSERT_EQ(game.game_updates(), count);
  // Total game time should be game updates * milliseconds_per_update.
  // 500 * 15 = 7500
  ASSERT_EQ(game.game_time(), std::chrono::milliseconds(7500));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
