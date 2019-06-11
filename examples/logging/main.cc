#include "plog/Log.h"

int main() {
  plog::init(plog::debug, "Hello.txt");

  PLOGD << "Hello!";
  PLOG_DEBUG << "Ugly Hello!";
  PLOG(plog::debug) << "Verbose hello!";

  return 0;
}
