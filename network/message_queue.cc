#include "message_queue.h"

#include <cassert>

namespace network {

void MessageQueue::Enqueue(uint8_t* data, int size) {
  assert(idx_ + size <= kBufferSize);
  std::lock_guard<std::mutex> lock(mutex_);
  memcpy(&buffer_[idx_], data, size);
  Message msg;
  msg.data = &buffer_[idx];
  msg.size = size;
  idx_ += size;
}

Message MessageQueue::Dequeue() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) return Message();  // Empty message
  auto data = queue_.front();
  queue_.pop();
  return data;
}

void MessageQueue::Reset() {
  queue_.clear();
  memset(buffer_, 0, events.idx);
}

bool MessageQueue::Empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

void MessageQueue::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  stop_ = true;
}

bool MessageQueue::IsStopped() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stop_;
}

}
