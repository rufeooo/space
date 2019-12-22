#include "window.h"

#include "math/vec.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@interface OpenGLView : NSView {
@private
  NSOpenGLContext* gl_context_;
}

  //@property (readonly, retain) NSOpenGLContext* gl_context;
- (id) init;
- (id) initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx;
- (BOOL) isOpaque;
- (BOOL) canBecomeKeyView;
- (BOOL) acceptsFirstResponder;
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
@end

@interface BaseWindow : NSWindow
- (void) close;

- (BOOL) acceptsFirstResponder;
@end

@implementation BaseWindow
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

namespace window
{

struct Window {
  // TODO: Verify this is needed. Likely doesn't matter though since
  // few allocations should be going on in here.
  NSAutoreleasePool* nspool;

  NSView* nsview;

  NSWindow* nswindow;

  NSOpenGLContext* gl_context;
};

static Window kWindow;

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


void Create(const char* name, int width, int height) {
  kWindow.gl_context = CreateOpenGLContext();

  unsigned int styleMask = NSTitledWindowMask | NSClosableWindowMask;
  kWindow.nsview = [[OpenGLView alloc]
                  initWithFrame:NSMakeRect(0,0, width, height)
                      glContext:kWindow.gl_context]; // TODO: Remove context.

  kWindow.nswindow = [[BaseWindow alloc]
                      initWithContentRect:[kWindow.nsview frame]
                                styleMask:styleMask
                                  backing:NSBackingStoreBuffered
                                    defer:NO];

  [kWindow.nswindow autorelease];
  [kWindow.nswindow setTitle:[NSString stringWithUTF8String:name]];
  [kWindow.nswindow setContentView:kWindow.nsview];
  [kWindow.nswindow makeFirstResponder:kWindow.nsview];
  [kWindow.nswindow center];
  [kWindow.nswindow makeKeyAndOrderFront:nil];
  [kWindow.nswindow setAcceptsMouseMovedEvents:YES];

  [kWindow.gl_context makeCurrentContext];
  [kWindow.gl_context setView:kWindow.nsview];
}

void TranslateEvent(NSEvent* nsevent, Event* event) {
  NSEventType nsevent_type = [nsevent type];
  switch (nsevent_type) {
    case NSEventTypeLeftMouseDown: {
      event->type = MOUSE_LEFT_DOWN;
    } break;
    case NSEventTypeLeftMouseUp: {
      event->type = MOUSE_LEFT_UP;
    } break;
    default:
      break;
  }
  NSPoint pos = [nsevent locationInWindow];
  event->position.x = pos.x;
  event->position.y = pos.y;
}

bool PollEvent(Event* event) {
  event->type = NOT_IMPLEMENTED;
  event->key = 0;
  event->position = math::Vec2f(0.f, 0.f);

  NSEvent* nsevent = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
  if (!nsevent) return false;
  // Convert the NSEvent* to an Event*
  TranslateEvent(nsevent, event);
  // Send the NSEvent nsevent to the app so it can handle it.
  [NSApp sendEvent:nsevent];
  return true;
}

void PollEvents() {
  NSEvent* event;
  // Drain event loop.
  while ((event =  [NSApp nextEventMatchingMask:NSEventMaskAny
                                      untilDate:[NSDate distantPast]
                                         inMode:NSDefaultRunLoopMode
                                        dequeue:YES])) {
    [NSApp sendEvent:event];
  }
}

void SwapBuffers() {
  [kWindow.gl_context flushBuffer];
}

bool ShouldClose() {
  return false;
}

math::Vec2f GetWindowSize() {
  // TODO: I have a feeling this is not the right size.
  NSSize size = [kWindow.nswindow minSize];
  return math::Vec2f(size.width, size.height);
}

math::Vec2f GetCursorPosition() {
  NSPoint pos;
  pos = [kWindow.nswindow mouseLocationOutsideOfEventStream];
  return math::Vec2f(pos.x, pos.y);
}
}  // namespace window
