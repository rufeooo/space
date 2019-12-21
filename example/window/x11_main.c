#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int running = 1;

void
signal_handler(int signal)
{
  running = 0;
}

int
main(int argc, char** argv)
{
  signal(SIGINT, signal_handler);

  Display* display = XOpenDisplay(NULL);
  int screen = DefaultScreen(display);
  Window root_window = RootWindow(display, screen);
  printf("%p\n", display);

  /*int gl_major = 0;
  int gl_minor = 0;
  int gl_version_ok = glXQueryVersion(display, &gl_major, &gl_minor);
  printf("gl version %d [ major %d ] [ minor %d ]\n", gl_version_ok, gl_major, gl_minor);*/

  int egl_major = 0;
  int egl_minor = 0;
  int egl_display = eglGetDisplay(display);
  int egl_version_ok = eglInitialize(egl_display, &egl_major, &egl_minor);
  int egl_error = eglGetError();
  printf("egl display %d egl version %d egl error 0x%x [ major %d ] [ minor %d ]\n", egl_display, egl_version_ok, egl_error, egl_major, egl_minor);

  Visual* visual = DefaultVisual(display, screen);
  int depth = DefaultDepth(display, screen);

  int colormap = XCreateColormap(display, root_window, visual, AllocNone);

  XSetWindowAttributes wa;
  const unsigned long wamask = CWBorderPixel | CWColormap | CWEventMask;

  wa.colormap = colormap;
  wa.border_pixel = 0;
  wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                  ExposureMask | FocusChangeMask | VisibilityChangeMask |
                  EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
  int border_width = 0;
  int x = 100, y = 100;
  int width = 400, height = 400;
  Window parent = root_window;
  int window_id =
      XCreateWindow(display, parent, x, y, width, height, border_width, depth,
                    InputOutput, CopyFromParent, wamask, &wa);
  printf("window %d\n", window_id);

  if (!window_id) {
    puts("fail XCreateWindow");
    exit(1);
  }

  while (running) usleep(100);

  XDestroyWindow(display, window_id);
  XCloseDisplay(display);
}
