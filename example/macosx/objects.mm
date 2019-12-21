#include <Foundation/Foundation.h>

@interface Box:NSObject {
  // Instance variables
  double length;
  double breadth;
  double height;
}

// Can access propertys via object.<property> syntax.
@property(nonatomic, readwrite) double height;
-(double) Volume;
@end

@implementation Box

@synthesize height;

-(id)
Initialize
{
  // init is defined in NSObject
  self = [super init];
  length = 1.0;
  breadth = 1.0;
  return self;
}

-(double)
Volume
{
  return length * breadth * height;
}

@end

int
main(int argc, char** argv)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  Box* box1 = [[Box alloc]Initialize];
  Box* box2 = [[Box alloc]Initialize];

  double volume = 0.0;

  // Property
  box1.height = 5.0;

  box2.height = 10.0;

  volume = [box1 Volume];
  NSLog(@"Volume Box1: %f", volume);

  volume = [box2 Volume];
  NSLog(@"Volume Box2: %f", volume);

  [pool drain];

  return 0;
}
