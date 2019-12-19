#include "filesystem.h"

#include <sys/stat.h>
#include <sys/types.h>

namespace filesystem
{
bool
MakeDirectory(const char* name)
{
  return mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

}  // namespace filesystem
