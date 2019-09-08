#pragma once

namespace observer {

// Base type for observer in observer pattern. A class would inherit
// from this if they are interested in receiving notifications of type
// M. For example -
//
// class InterestingObserver : Observer<Foo> {
//  public:
//   void OnNotification(const Foo& foo) override {
//     // Called when subject calls Notify(...).
//   }
// };
template <class M>
class Observer {
 public:
  // You are responsible for calling Detach on the subject before the
  // observer is destroyed.
  virtual ~Observer() = default;
  virtual void OnNotification(const M& m) = 0;
};

}  // observer
