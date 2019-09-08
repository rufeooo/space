#pragma once

#include <vector>

#include "observer.h"

namespace observer {

template <class M>
class Subject {
 public:
  void Notify(const M& m) {
    for (auto& o : observers_) {
      o->OnNotification(m);
    }
  }

  void Notify(M&& m) {
    for (auto& o : observers_) {
      o->OnNotification(m);
    }
  }

  bool Attach(Observer<M>* observer) {
    observers_.push_back(observer);
    return true;
  }

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
