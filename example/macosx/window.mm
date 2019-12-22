#include <cassert>

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

NSOpenGLContext*
CreateOpenGLContext()
{
  CGLError error;
  CGLPixelFormatAttribute pixel_attrs[] = {
      kCGLPFADoubleBuffer,
      kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
      kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
      kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
      kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
      kCGLPFAStencilSize, (CGLPixelFormatAttribute)8,
      kCGLPFASampleBuffers, (CGLPixelFormatAttribute)0,
      (CGLPixelFormatAttribute) 0,
  };

  int ignore;
  CGLPixelFormatObj pixel_format;
  error = CGLChoosePixelFormat(pixel_attrs, &pixel_format, &ignore);
  assert(!error);
  assert(pixel_format);

  // Create the GL context.
  CGLContextObj context;
  error = CGLCreateContext(pixel_format, 0, &context);
  assert(!error);
  assert(context);

  return [[NSOpenGLContext alloc] initWithCGLContextObj:context];
}

@interface MyWindow : NSWindow
- (void) close;

- (BOOL) acceptsFirstResponder;
@end

@implementation MyWindow
// Close the window - https://developer.apple.com/documentation/appkit/nswindow/1419662-close?language=objc
// This also terminates the NSApplication 
- (void)
close
{
  [NSApp terminate:self];
  // If the app refused to terminate, this window should still close.
  [super close];
}

// The receiver is the first object in the responder chain to be sent
// key events and action messages.
// https://developer.apple.com/documentation/appkit/nsresponder/1528708-acceptsfirstresponder?language=objc
- (BOOL)
acceptsFirstResponder
{
  return YES;
}
@end

int
main(int argc, const char** argv)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  // Every app has a single instance of NSApplication.
  // It controls the main event loop, keeps track of app's windows
  // and menus, distribute events...
  // https://developer.apple.com/documentation/appkit/nsapplication?language=objc
  // This call returns a pointer to the shared application or you can use
  // the NSApp global as I do in this example...
  //[NSApplication sharedApplication];

  unsigned int styleMask = NSTitledWindowMask
                           | NSClosableWindowMask;

  NSOpenGLContext* gl_context = CreateOpenGLContext();

  NSWindow* window = [[MyWindow alloc]
                      initWithContentRect:NSMakeRect(0,0, 500, 500)
                      styleMask:styleMask
                      backing:NSBackingStoreBuffered
                      defer:NO];

  //[application setDelegate:window];
  [window makeKeyAndOrderFront:NSApp];
  [window autorelease];
  [window setTitle:@"Title"];
  //[window setContentView:contentView];
  //[window makeFirstResponder:contentView];
  [window center];
  // This is required... This actually runs the event loop associated with my
  // window.
  [NSApp run];
  [pool drain];
  return 0;
}
