#pragma once

#include "gmock/gmock.h"
#include "observer.h"

namespace observer {

template <class M>
class MockObserver : public Observer<M> {
public:
  MOCK_METHOD(void, OnNotification, (const M&), (override));
};

}  // observer
