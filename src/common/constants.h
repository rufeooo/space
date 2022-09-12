// clang-format off
#pragma once

// Players in one game
#define MAX_PLAYER 2

// Invalid offset into an array type 
// Equivalent to memset(0xff) for up to 64 bits
#define kInvalidIndex ((uint64_t){-1})

// Invalid unique identifier ('id') for assigning stable identification numbers
// Equivalent to memset(0x00) for up to 64 bits
// Allows for boolean tests: (id ? "exists: "not-exists")
#define kInvalidId ((uint64_t){0})

#define kMaxBlackboardItems 16
#define kMaxBlackboardValueSize 64
