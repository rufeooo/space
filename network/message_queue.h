#pragma once

#include <mutex>
#include <queue>
#include <cassert>

namespace network {

constexpr int kMaxMessageSize = 1024;

struct Message {
  char* data;
  int size;
};

static Message kEmptyMessage;

class MessageQueue {
 public:
  // Just exists to test.
  void Enqueue(const std::string& str) {
    Message msg;
    msg.data = (char*)malloc(str.size());
    strncpy(msg.data, str.c_str(), str.size());
    msg.size = str.size();
    Enqueue(str);
  }

  void Enqueue(Message message) {
    assert(message.size <= kMaxMessageSize);
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(message);
  }

  Message Dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return kEmptyMessage;
    auto data = queue_.front();
    queue_.pop();
    return data;
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  void Stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }

  bool IsStopped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stop_;
  };
 private:
  mutable std::mutex mutex_;
  std::queue<Message> queue_;
  bool stop_ = false;
};

}
