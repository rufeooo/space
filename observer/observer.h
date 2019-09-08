#pragma once

namespace observer {

template <class M>
class Observer {
 public:
  virtual ~Observer() = default;
  virtual void OnNotification(const M& m) = 0;
};

}  // observer
