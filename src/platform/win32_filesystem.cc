#include "filesystem.h"

#include <Windows.h>

namespace filesystem
{
bool
MakeDirectory(const char* name)
{
  return CreateDirectoryA(name, nullptr);
}

}  // namespace filesystem
