#include <cassert>

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

static const int kSpaceKey = 49;

//https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_pg_concepts/opengl_pg_concepts.html#//apple_ref/doc/uid/TP40001987-CH208-SW1 
// Worthy read of OpenGL on this wonderful platform.
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

@interface Window : NSWindow
- (void) close;

- (BOOL) acceptsFirstResponder;
@end

@implementation Window
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

//https://developer.apple.com/documentation/appkit/nsview?language=objc
// Required to give the opengl context, or framebuffer, access to the windows view. 
@interface OpenGLView : NSView {
@private
  NSOpenGLContext* gl_context_;
}

  //@property (readonly, retain) NSOpenGLContext* gl_context;
- (id) init;
- (id) initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx;
- (void) drawRect:(NSRect)bounds;
- (BOOL) isOpaque;
- (BOOL) canBecomeKeyView;
- (BOOL) acceptsFirstResponder;
- (void) mouseDown:(NSEvent*)event;

//! Call drawRect if space key is received.
- (void) keyDown:(NSEvent*)event;
@end

@implementation OpenGLView
- (id)
init
{
  return nil;
}

- (id)
initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx
{
  self = [super initWithFrame:rect];
  if (self == nil)
      return nil;
  gl_context_ = ctx;
  return self;
}

- (BOOL)
isOpaque
{
  return YES;
}

- (BOOL)
canBecomeKeyView
{
  return YES;
}

- (BOOL)
acceptsFirstResponder
{
  return YES;
}

- (void)
mouseDown:(NSEvent*)event
{
  [self drawRect:[self frame]];
}

- (void)
keyDown:(NSEvent*)event
{
  if ([event keyCode] == kSpaceKey) {
    [self drawRect:[self frame]];
  } else {
    [super keyDown:event];
  }
}

- (void)
drawRect:(NSRect)bounds
{
  static int i = 0;
  i = ++i % 3;

  switch (i) {
    case 0: glClearColor(1, 0, 0, 1); break;
    case 1: glClearColor(0, 1, 0, 1); break;
    case 2: glClearColor(0, 0, 1, 1); break;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  [gl_context_ flushBuffer];
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

  NSView* view = [[OpenGLView alloc]
                  initWithFrame:NSMakeRect(0,0, 500, 500)
                      glContext:gl_context];
                  
  NSWindow* window = [[Window alloc]
                      initWithContentRect:[view frame]
                                styleMask:styleMask
                                  backing:NSBackingStoreBuffered
                                    defer:NO];

  // Setup window
  [window autorelease];
  [window setTitle:@"Title"];
  [window setContentView:view];
  [window makeFirstResponder:view];
  [window center];
  [window makeKeyAndOrderFront:nil];

  // Setup gl view.
  [gl_context makeCurrentContext];
  [gl_context setView:view];
  [view display];

  // This is required... This actually runs the event loop associated with my
  // window.
  [NSApp run];
  [pool drain];
  return 0;
}
