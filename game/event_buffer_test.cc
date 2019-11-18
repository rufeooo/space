#include <iostream>

#include "gtest/gtest.h"
#include "event_buffer.h"

TEST(EventBuffer, HappyPathEventBuffer) {
  game::AllocateEventBuffer(1024);

  struct Foo {
    int a;
    int b;
    int c;
  };
  Foo* foo = (Foo*)game::EnqueueEvent(sizeof(Foo), 0);
  foo->a = 1;
  foo->b = 2;
  foo->c = 3;

  struct Bar {
    uint64_t a;
    double b;
  };
  Bar* bar = (Bar*)game::EnqueueEvent(sizeof(Bar), 1);
  bar->a = 45;
  bar->b = 5.0;

  struct Baz {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
  };
  Baz* baz = (Baz*)game::EnqueueEvent(sizeof(Baz), 2);
  baz->a = 1;
  baz->b = 2;
  baz->c = 3;
  baz->d = 4;
  baz->e = 5;

  game::Event event;

  game::PollEvent(&event);
  ASSERT_EQ(event.size, sizeof(Foo));
  ASSERT_EQ(event.metadata, 0);
  ASSERT_EQ(((Foo*)event.data)->a, 1);
  ASSERT_EQ(((Foo*)event.data)->b, 2);
  ASSERT_EQ(((Foo*)event.data)->c, 3);

  game::PollEvent(&event);
  ASSERT_EQ(event.size, sizeof(Bar));
  ASSERT_EQ(event.metadata, 1);
  ASSERT_EQ(((Bar*)event.data)->a, 45);
  ASSERT_EQ(((Bar*)event.data)->b, 5.0);

  game::PollEvent(&event);
  ASSERT_EQ(event.size, sizeof(Baz));
  ASSERT_EQ(event.metadata, 2);
  ASSERT_EQ(((Baz*)event.data)->a, 1);
  ASSERT_EQ(((Baz*)event.data)->b, 2);
  ASSERT_EQ(((Baz*)event.data)->c, 3);
  ASSERT_EQ(((Baz*)event.data)->d, 4);
  ASSERT_EQ(((Baz*)event.data)->e, 5);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
