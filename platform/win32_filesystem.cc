#include "filesystem.h"

#include <Windows.h>

namespace filesystem {

bool MakeDirectory(const char* name) {
    return CreateDirectory(name, nullptr);
}

}
