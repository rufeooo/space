#pragma once

#include <mutex>
#include <queue>
#include <cassert>

#include <flatbuffers/flatbuffers.h>

namespace network {

constexpr int kMaxMessageSize = 1024;

struct Message {
  char* data = nullptr;
  int size = 0;
};

class IncomingMessageQueue {
 public:
  void Enqueue(Message message) {
    assert(message.size <= kMaxMessageSize);
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(message);
  }

  Message Dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return Message();  // Empty message
    auto data = queue_.front();
    assert(data.size <= kMaxMessageSize);
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

class OutgoingMessageQueue {
 public:
  void Enqueue(flatbuffers::DetachedBuffer&& message) {
    //assert(message.size <= kMaxMessageSize);
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(message));
  }

  flatbuffers::DetachedBuffer Dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return flatbuffers::DetachedBuffer();
    auto data = std::move(queue_.front());
    //assert(data.size <= kMaxMessageSize);
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
  std::queue<flatbuffers::DetachedBuffer> queue_;
  bool stop_ = false;
};

}
