#include <OpenGL/gl3.h>
#include <iostream>

#import <Cocoa/Cocoa.h>
#import <memory>


NSApplication* application;

static bool kTerminate = false;

@interface MacApp : NSWindow <NSApplicationDelegate>{
}

@property (nonatomic, retain) NSOpenGLView* glView;

@end

@implementation MacApp

-
(id)
initWithContentRect:(NSRect)contentRect
                     styleMask:(NSUInteger)aStyle
                     backing:(NSBackingStoreType)bufferingType
                     defer:(BOOL)flag
{
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

- (BOOL)
applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
  std::cout << "applicationShouldTerminateAfterLastWindowClosed" << std::endl;
  kTerminate = true;
  return YES;
}

@end

//@synthesize glView;

int
main(int argc, const char * argv[])
{
  MacApp* app;
  application = [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular]; 
  //create a window with the size of 600 by 600   
  app = [[MacApp alloc] initWithContentRect:NSMakeRect(0, 0, 600, 600)
         styleMask:NSTitledWindowMask | NSClosableWindowMask |  NSMiniaturizableWindowMask
         backing:NSBackingStoreBuffered defer:YES];    
  [application setDelegate:app];

  // Not really sure why I need this or if I need this.
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  while (!kTerminate) {
    NSEvent* event;
    while ((event =  [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES])) {
      NSEventType event_type = [event type];
      switch (event_type) {
        case NSLeftMouseDown: {
          // TODO: How to get mouse position?
          NSLog(@"Left mouse down");
        } break;
        case NSKeyDown: {
          NSString* characters; 
          characters = [event charactersIgnoringModifiers];
          unsigned int characterIndex, characterCount;
          characterCount = [characters length]; 
          for (characterIndex = 0; characterIndex < characterCount; characterIndex++) {
            unichar c = [characters characterAtIndex:characterIndex];
            std::cout << c << std::endl;
          }
        } break;
        default:
          break;
      }
      // Send event to application so the user can do things like close the window.
      [application sendEvent:event];
    }

    //glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    [application updateWindows];
  }

  [pool release];

  return 0;
}
