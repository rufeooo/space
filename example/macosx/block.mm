#include <Foundation/Foundation.h>

// Block == function pointer....
typedef void (^CompletionBlock)();
@interface SampleClass:NSObject
- (void)PerformActionWithCompletion:(CompletionBlock)completion_block;
@end

@implementation SampleClass
- (void)
PerformActionWithCompletion:
(CompletionBlock)completion_block
{
  NSLog(@"Action performed...");
  completion_block();
}
@end

int
main(int argc, char** argv)
{
  void (^simple_block)(void) = ^{
    NSLog(@"This is a block.");
  };
  simple_block();
  SampleClass* sample_class = [[SampleClass alloc]init];
  // But can use like a lambda.
  [sample_class PerformActionWithCompletion:^{
    NSLog(@"Completion...");
  }];
  return 0;
}
