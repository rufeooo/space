#pragma once

#include <mutex>
#include <queue>

namespace network {

constexpr int kBufferSize = 4096;

struct Message {
  // Pointer to the message to send.
  uint8_t* data = nullptr;
  // Size of said message.
  int size = 0;
};

class MessageQueue {
 public:
  void Enqueue(uint8_t* data, int size);
  Message Dequeue();

  void Reset();

  bool Empty() const;
  void Stop();
  bool IsStopped() const;

 private:
  mutable std::mutex mutex_;

  std::queue<Message> queue_;
  bool stop_ = false;

  uint8_t buffer_[kBufferSize];
  int idx_ = 0;
};

}
