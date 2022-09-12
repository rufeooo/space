#pragma once

static uint8_t kEmptyValue[kMaxBlackboardValueSize];

STATIC bool
blackboard_test(Blackboard* bb, uint64_t idx)
{
  assert(idx < kMaxBlackboardItems);
  return memcmp(&bb->value[idx], &kEmptyValue, kMaxBlackboardValueSize) != 0;
}

STATIC bool
blackboard_set(Blackboard* bb, uint64_t idx, const uint8_t* bytes,
               uint64_t size)
{
  assert(idx < kMaxBlackboardItems);
  assert(size < kMaxBlackboardValueSize);
  memcpy(&bb->value[idx], bytes, size);
  return true;
}

STATIC bool
blackboard_get(Blackboard* bb, uint64_t idx, const uint8_t** bytes)
{
  assert(idx < kMaxBlackboardItems);
  if (!blackboard_test(bb, idx)) return false;
  *bytes = bb->value[idx];
  return true;
}

STATIC void
blackboard_remove(Blackboard* bb, uint64_t idx)
{
  assert(idx < kMaxBlackboardItems);
  memcpy(&bb->value[idx], &kEmptyValue, kMaxBlackboardValueSize);
}

#define BB_SET(bb, idx, val) \
  blackboard_set(bb, idx, reinterpret_cast<const uint8_t*>(&val), sizeof(val))
#define BB_GET(bb, idx, ptr) \
  blackboard_get(bb, idx, reinterpret_cast<const uint8_t**>(&ptr))
#define BB_EXI(bb, idx) blackboard_test(bb, idx)
#define BB_REM(bb, idx) blackboard_remove(bb, idx)
