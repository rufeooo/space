#import <Cocoa/Cocoa.h>

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
  [window makeKeyAndOrderFront:nil];
  // This is required... This actually runs the event loop associated with my
  // window.
  [NSApp run];
  [pool drain];
  return 0;
}
