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

-(id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag{
  if(self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag]){
    //sets the title of the window (Declared in Plist)
    [self setTitle:[[NSProcessInfo processInfo] processName]];
 
    //This is pretty important.. OS X starts always with a context that only supports openGL 2.1
    //This will ditch the classic OpenGL and initialises openGL 4.1
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] ={
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            0
    };

    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc]initWithAttributes:pixelFormatAttributes];
    //Initialize the view 
    _glView = [[NSOpenGLView alloc]initWithFrame:contentRect pixelFormat:format];
    
    //Set context and attach it to the window
    [[_glView openGLContext]makeCurrentContext];
  
    //finishing off
    [self setContentView:_glView];
    [_glView prepareOpenGL];
    [self makeKeyAndOrderFront:self];
    [self setAcceptsMouseMovedEvents:YES];
    [self makeKeyWindow];
    [self setOpaque:YES];
    //Start the c++ code
    //appInstance = std::shared_ptr<Application>(new Application());
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
  [application run];
  return 0;
}
