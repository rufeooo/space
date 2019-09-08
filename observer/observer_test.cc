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

struct FooMessage {
  FooMessage(int state) : state(state) {}
  bool operator==(const FooMessage& rhs) const {
    return state == rhs.state;
  }
  int state;
};

struct BarMessage {
  BarMessage(int state) : state(state) {}
  bool operator==(const BarMessage& rhs) const {
    return state == rhs.state;
  }
  int state;
};

class FooBarSubject :
    public observer::Subject<FooMessage>,
    public observer::Subject<BarMessage> {
 public:
  USING_SUBJECT_CALLS(FooMessage)
  USING_SUBJECT_CALLS(BarMessage)

  void SetFooState(int n) {
    Notify(FooMessage(n));
  }

  void SetBarState(int n) {
    Notify(BarMessage(n));
  }
};

TEST(ObserverTest, SubjectWithMultipleMessages) {
  observer::MockObserver<FooMessage> mock_foo_observer;
  observer::MockObserver<BarMessage> mock_bar_observer;
  FooBarSubject subject;
  EXPECT_TRUE(subject.Attach(&mock_foo_observer));
  EXPECT_TRUE(subject.Attach(&mock_bar_observer));
  EXPECT_CALL(mock_foo_observer,
              OnNotification(::testing::Eq(FooMessage(1)))).Times(1);
  subject.SetFooState(1);
  EXPECT_CALL(mock_bar_observer,
              OnNotification(::testing::Eq(BarMessage(2)))).Times(1);
  subject.SetBarState(2);
  EXPECT_CALL(mock_foo_observer,
              OnNotification(::testing::Eq(FooMessage(2))))
      .Times(1);
  subject.SetFooState(2);
  EXPECT_CALL(mock_bar_observer,
              OnNotification(::testing::Eq(BarMessage(3))))
      .Times(1);
  subject.SetBarState(3);
  EXPECT_TRUE(subject.Detach(&mock_foo_observer));
  EXPECT_CALL(mock_foo_observer, OnNotification(::testing::_))
      .Times(0);
  EXPECT_CALL(mock_bar_observer,
              OnNotification(::testing::Eq(BarMessage(2))))
      .Times(1);
  subject.SetFooState(1);
  subject.SetBarState(2);
  EXPECT_TRUE(subject.Detach(&mock_bar_observer));
  EXPECT_CALL(mock_bar_observer, OnNotification(::testing::_))
      .Times(0);
  subject.SetFooState(1);
  subject.SetBarState(2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
