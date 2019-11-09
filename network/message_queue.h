#pragma once

#include <mutex>
#include <queue>
#include <cassert>
#include <unordered_set>
#include <vector>

#include <flatbuffers/flatbuffers.h>

namespace network {

constexpr int kMaxMessageSize = 1024;

struct Message {
  uint8_t* data = nullptr;
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

  void AddRecipient(int client_id);
  void RemoveRecipient(int client_id);

  std::vector<int> Recipients() const;
  std::vector<int> NewRecipients();

 private:
  mutable std::mutex mutex_;
  std::queue<flatbuffers::DetachedBuffer> queue_;
  bool stop_ = false;
  std::unordered_set<int> recipients_;
  std::unordered_set<int> new_recipients_;
};

}
