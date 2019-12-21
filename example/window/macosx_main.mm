#include <OpenGL/gl3.h>
#include <iostream>

#import <Cocoa/Cocoa.h>
#import <memory>

NSApplication* application;

@interface MacApp : NSWindow <NSApplicationDelegate>{
}

@property (nonatomic, retain) NSOpenGLView* glView;

@end

@implementation MacApp

-(id)initWithContentRect:(NSRect)contentRect
                         styleMask:(NSUInteger)aStyle
                         backing:(NSBackingStoreType)bufferingType
                         defer:(BOOL)flag {
  if(self = [super initWithContentRect:contentRect
                   styleMask:aStyle
                   backing:bufferingType
                   defer:flag]) {
    [self setTitle:[[NSProcessInfo processInfo] processName]];

    // This initializes GL to 4.1 WHY.
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] ={
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            0
    };

    NSOpenGLPixelFormat* format =
        [[NSOpenGLPixelFormat alloc]initWithAttributes:pixelFormatAttributes];
    _glView =
        [[NSOpenGLView alloc]initWithFrame:contentRect pixelFormat:format];
    [[_glView openGLContext]makeCurrentContext];
  
    [self setContentView:_glView];
    [_glView prepareOpenGL];
    [self makeKeyAndOrderFront:self];
    [self setAcceptsMouseMovedEvents:YES];
    [self makeKeyWindow];
    [self setOpaque:YES];
  }
  return self;
}

@end

//@synthesize glView;

BOOL shouldStop = NO;

int main(int argc, const char * argv[]) {
  MacApp* app;
  application = [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular]; 
  //create a window with the size of 600 by 600   
  app = [[MacApp alloc] initWithContentRect:NSMakeRect(0, 0, 600, 600)
         styleMask:NSTitledWindowMask | NSClosableWindowMask |  NSMiniaturizableWindowMask
         backing:NSBackingStoreBuffered defer:YES];    

  std::cout << glGetString(GL_VERSION) << std::endl;

  [application setDelegate:app];

  // Not really sure why I need this or if I need this.
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  bool shouldKeepRunning = true;
  do
  {
    [pool release];
    pool = [[NSAutoreleasePool alloc] init];

    NSEvent *event =
        [application
            nextEventMatchingMask:NSAnyEventMask
            untilDate:[NSDate distantFuture]
            inMode:NSDefaultRunLoopMode
            dequeue:YES];

    [application sendEvent:event];
    [application updateWindows];

    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  } while (shouldKeepRunning);

  [pool release];

  return 0;
}
