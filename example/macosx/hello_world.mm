// NSArray, NSDictionary, NSSet...
#include <Foundation/Foundation.h>

int
main(int argc, const char** argv)
{
  // Supports reference-counted memory management.
  // Even though I may not want to use it cocoa library uses it extensively
  // so I need to prepare the pool. Autoreleased objects will leak if I do
  // not do this.
  // https://developer.apple.com/documentation/foundation/nsautoreleasepool
  // Deeper explanation
  // https://www.mikeash.com/pyblog/friday-qa-2011-09-02-lets-build-nsautoreleasepool.html
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSLog(@"Hello world");
  [pool drain];
  return 0;
}
