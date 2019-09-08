#include "gtest/gtest.h"

#include "mock_observer.h"
#include "subject.h"

struct Message {
  Message(int state) : state(state) {}
  bool operator==(const Message& rhs) const {
    return state == rhs.state;
  }
  int state;
};

class BasicSubject : public observer::Subject<Message> {
 public:
  void SetState(int n) {
    Notify(Message(n));
  }
};

TEST(ObserverTest, BasicNotificationsSingleObserver) {
  observer::MockObserver<Message> mock_observer;
  BasicSubject subject;
  EXPECT_TRUE(subject.Attach(&mock_observer));
  EXPECT_CALL(mock_observer, OnNotification(::testing::Eq(Message(1))))
      .Times(1);
  subject.SetState(1);
  EXPECT_CALL(mock_observer, OnNotification(::testing::Eq(Message(2))))
      .Times(1);
  subject.SetState(2);
  EXPECT_TRUE(subject.Detach(&mock_observer));
  EXPECT_CALL(mock_observer, OnNotification(::testing::_))
      .Times(0);
  subject.SetState(1);
}

TEST(ObserverTest, BasicNotificationsMultipleObservers) {
  observer::MockObserver<Message> mock_observer1, mock_observer2;
  BasicSubject subject;
  EXPECT_TRUE(subject.Attach(&mock_observer1));
  EXPECT_TRUE(subject.Attach(&mock_observer2));
  EXPECT_CALL(mock_observer1,
              OnNotification(::testing::Eq(Message(1))))
      .Times(1);
  EXPECT_CALL(mock_observer2,
              OnNotification(::testing::Eq(Message(1))))
      .Times(1);
  subject.SetState(1);
  EXPECT_CALL(mock_observer1,
              OnNotification(::testing::Eq(Message(2))))
      .Times(1);
  EXPECT_CALL(mock_observer2,
              OnNotification(::testing::Eq(Message(2))))
      .Times(1);
  subject.SetState(2);
  EXPECT_TRUE(subject.Detach(&mock_observer1));
  EXPECT_TRUE(subject.Detach(&mock_observer2));
  EXPECT_CALL(mock_observer1, OnNotification(::testing::_)).Times(0);
  EXPECT_CALL(mock_observer2, OnNotification(::testing::_)).Times(0);
  subject.SetState(1);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
