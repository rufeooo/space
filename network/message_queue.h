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
  int client_id = -1;
};

class IncomingMessageQueue {
 public:
  void Enqueue(Message message);
  Message Dequeue();
  bool Empty() const;
  void Stop();
  bool IsStopped() const;

 private:
  mutable std::mutex mutex_;
  std::queue<Message> queue_;
  bool stop_ = false;
};

class OutgoingMessageQueue {
 public:
  void Enqueue(flatbuffers::DetachedBuffer&& message);
  flatbuffers::DetachedBuffer Dequeue();
  bool Empty() const;
  void Stop();
  bool IsStopped() const;

 private:
  mutable std::mutex mutex_;
  std::queue<flatbuffers::DetachedBuffer> queue_;
  bool stop_ = false;
};

}
