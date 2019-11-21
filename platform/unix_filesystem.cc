#include "filesystem.h"

#include <sys/types.h>
#include <sys/stat.h>

namespace filesystem {

bool MakeDirectory(const char* name) {
  return mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

}
