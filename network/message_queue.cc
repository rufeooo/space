#include "message_queue.h"

namespace network {

void IncomingMessageQueue::Enqueue(Message message) {
  assert(message.size <= kMaxMessageSize);
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(message);
}

Message IncomingMessageQueue::Dequeue() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) return Message();  // Empty message
  auto data = queue_.front();
  assert(data.size <= kMaxMessageSize);
  queue_.pop();
  return data;
}

bool IncomingMessageQueue::Empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

void IncomingMessageQueue::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  stop_ = true;
}

bool IncomingMessageQueue::IsStopped() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stop_;
}

void OutgoingMessageQueue::Enqueue(
    flatbuffers::DetachedBuffer&& message) {
  assert(message.size() <= kMaxMessageSize);
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(std::move(message));
}

flatbuffers::DetachedBuffer OutgoingMessageQueue::Dequeue() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (queue_.empty()) return flatbuffers::DetachedBuffer();
  auto data = std::move(queue_.front());
  assert(data.size() <= kMaxMessageSize);
  queue_.pop();
  return data;
}

bool OutgoingMessageQueue::Empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}

void OutgoingMessageQueue::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  stop_ = true;
}

bool OutgoingMessageQueue::IsStopped() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stop_;
}

}
