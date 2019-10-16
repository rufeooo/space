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

void OutgoingMessageQueue::AddRecipient(int client_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  recipients_.insert(client_id);
}

void OutgoingMessageQueue::RemoveRecipient(int client_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto found = recipients_.find(client_id);
  if (found == recipients_.end()) return;
  recipients_.erase(client_id);
}

std::vector<int> OutgoingMessageQueue::Recipients() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return std::vector<int>(recipients_.begin(), recipients_.end());
}

}
