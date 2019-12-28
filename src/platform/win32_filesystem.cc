#include "filesystem.h"

#include <windows.h>

namespace filesystem
{
bool
MakeDirectory(const char* name)
{
  return CreateDirectoryA(name, nullptr);
}

}  // namespace filesystem
