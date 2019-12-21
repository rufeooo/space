#include <Foundation/Foundation.h>

// Interface is wrapped in @interface @end semantics
// All objects inherit from NSObject? Great...
@interface SampleClass:NSObject
// Functions start with return type in parens then name
- (void)SomeMethod;
@end

// Implementation to interface above @implementation / @end
@implementation SampleClass
-
(void)
SomeMethod
{
  NSLog(@"This really needed to be a class, clearly.\n");
}
@end

int main(int argc, char** argv) {
  // Why no pool?
  // Memory management in Objective C has two methods -
  // MRR - Mutual Retail Release.
  // ARC - Automatic reference counting.
  // We own anything we create using -
  //   alloc, init, copy, mutableCopy
  SampleClass* sample_class = [[SampleClass alloc]init]; // Ref count 1
  // Comment this out and second release will fail...
  [sample_class retain];                                 // Ref count 2
  // Functions are called with this bracket notation.
  // [*pointer <function_name>]
  [sample_class SomeMethod];
  // Automatically release this?
  [sample_class release]; // Ref count 1
  [sample_class release]; // Ref count 0
  return 0;
}
