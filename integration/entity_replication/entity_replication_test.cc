#include "gtest/gtest.h"

#include "entity_replication.h"

void AssertRemoveContainsEntities(
    std::vector<ecs::Entity> entities,
    const integration::EntitySystemDelta& delta) {
  for (ecs::Entity entity : entities) {
    bool found = false;
    for (const auto& delta_remove : delta.remove) {
      if (delta_remove == entity) found = true;
    }
    ASSERT_TRUE(found);
  }
}


void AssertAddContainsEntities(
    std::vector<ecs::Entity> entities,
    const integration::EntitySystemDelta& delta) {
  for (ecs::Entity entity : entities) {
    bool found = false;
    for (const auto& delta_add : delta.add) {
      if (delta_add.first == entity) found = true;
    }
    ASSERT_TRUE(found);
  }
}

// Authority: 1 2 3
// Client 0 :   2   4 5
// Delta    : client_id: 0 { remove(4, 5), add(1, 3) }
TEST(EntityReplication, EntityReplicationHappyPath) {
  integration::EntityReplication replication;
  replication.AddEntityAuthority(1, std::vector<uint8_t>());
  replication.AddEntityAuthority(2, std::vector<uint8_t>());
  replication.AddEntityAuthority(3, std::vector<uint8_t>());
  replication.AddEntityClient(0, 2);
  replication.AddEntityClient(0, 4);
  replication.AddEntityClient(0, 5);
  replication.Run();
  integration::EntitySystemDelta delta;
  ASSERT_TRUE(replication.PollDeltas(&delta));
  ASSERT_EQ(delta.client_id, 0);
  AssertRemoveContainsEntities({4, 5}, delta);
  AssertAddContainsEntities({1, 3}, delta);
  ASSERT_FALSE(replication.PollDeltas(&delta));
  // Apply the delta.
  replication.ApplyDelta(delta);
  // Now running replication should produce no deltas.
  replication.Run();
  ASSERT_FALSE(replication.PollDeltas(&delta));
}

// Authority: 1 2 3
// Client 0 :   2   4 5
// Client 1 : 1   3 10 11 
// Delta    : client_id: 0 { remove(4, 5), add(1, 3) }
//            client_id: 1 { remove(10, 11), add(2) }
TEST(EntityReplication, EntityReplicationMultipleClients) {
  integration::EntityReplication replication;
  replication.AddEntityAuthority(1, std::vector<uint8_t>());
  replication.AddEntityAuthority(2, std::vector<uint8_t>());
  replication.AddEntityAuthority(3, std::vector<uint8_t>());
  replication.AddEntityClient(0, 2);
  replication.AddEntityClient(0, 4);
  replication.AddEntityClient(0, 5);
  replication.AddEntityClient(1, 1);
  replication.AddEntityClient(1, 3);
  replication.AddEntityClient(1, 10);
  replication.AddEntityClient(1, 11);
  replication.Run();
  integration::EntitySystemDelta delta;
  ASSERT_TRUE(replication.PollDeltas(&delta));
  bool checked_zero = false;
  bool checked_one = false;
  if (delta.client_id == 0) {
    AssertRemoveContainsEntities({4, 5}, delta);
    AssertAddContainsEntities({1, 3}, delta);
    checked_zero = true;
  } else {
    AssertRemoveContainsEntities({10, 11}, delta);
    AssertAddContainsEntities({2}, delta);
    checked_one = true;
  }
  ASSERT_TRUE(replication.PollDeltas(&delta));
  if (delta.client_id == 0) {
    AssertRemoveContainsEntities({4, 5}, delta);
    AssertAddContainsEntities({1, 3}, delta);
    checked_zero = true;
  } else {
    AssertRemoveContainsEntities({10, 11}, delta);
    AssertAddContainsEntities({2}, delta);
    checked_one = true;
  }
  ASSERT_TRUE(checked_zero);
  ASSERT_TRUE(checked_one);
}

// Authority: 3 6 8
// Client 0 : 3 6 8
// Delta    : <none>
TEST(EntityReplication, EntityReplicationMatchingEntities) {
  integration::EntityReplication replication;
  replication.AddEntityAuthority(3, std::vector<uint8_t>());
  replication.AddEntityAuthority(6, std::vector<uint8_t>());
  replication.AddEntityAuthority(8, std::vector<uint8_t>());
  replication.Run();
  integration::EntitySystemDelta delta;
  ASSERT_FALSE(replication.PollDeltas(&delta));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
