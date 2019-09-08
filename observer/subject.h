// Subject for observer pattern. Multiple observers will attach to a
// subject and receive notifications of type M.

#pragma once

#include <vector>

#include "observer.h"

namespace observer {

// Required when inheriting from multiple subjects to distinguish
// between different templated calls. If the subject inherits from a 
// single type this macro is not required.
#define USING_SUBJECT_CALLS(Message) \
  using observer::Subject<Message>::Notify; \
  using observer::Subject<Message>::Attach; \
  using observer::Subject<Message>::Detach; \

// Base type for subject of observer pattern. A class would inherit
// from this type if they are interested in broadcasting messages to
// interested observers. For example -
//
// class InterestingSubject : Subject<Foo>, Subject<Bar> {
//  public:
//   USING_SUBJECT_CALLS(Foo)
//   USING_SUBJECT_CALLS(Bar)
//
//   void PerformInterestingOperation() {
//     // Do interesting stuff.
//     ...
//     // Notify Foo observers of interesting stuff.
//     Notify(Foo(...)); 
//   }
//
//   void PerformAnotherInterestingOperation() {
//     // Do interesting stuff.
//     ...
//     // Notify Bar observer of interesting stuff.
//     Notify(Bar(...));
//   }
// };
template <class M>
class Subject {
 public:
  // Notify all observers of message M.
  void Notify(const M& m) {
    for (auto& o : observers_) {
      o->OnNotification(m);
    }
  }

  // Notify all observers of message M with move semantics.
  void Notify(M&& m) {
    for (auto& o : observers_) {
      o->OnNotification(m);
    }
  }

  // Attach to the subject so an observer may begin to receive
  // notifications.
  bool Attach(Observer<M>* observer) {
    observers_.push_back(observer);
    return true;
  }

  // Detach so an observer no longer recieves updates from the subject.
  bool Detach(Observer<M>* observer) {
    auto found = std::find(
        observers_.begin(), observers_.end(), observer);
    if (found == observers_.end()) return false;
    observers_.erase(found);
    return true;
  }

 private:
  std::vector<Observer<M>*> observers_;
};

}  // observer
