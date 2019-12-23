#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"

volatile int running = 1;

void
signal_handler(int signal)
{
  running = 0;
}

int
x11_error_handler(Display* d, XErrorEvent* ev)
{
  printf("X11 Error! %d", ev ? ev->type : 0);
  return 0;
}

int
x11_ioerror_handler(Display* d)
{
  puts("X11 IO error!");
  return 0;
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
  printf("gl version %d [ major %d ] [ minor %d ]\n", gl_version_ok, gl_major,
  gl_minor);*/

  int egl_major = 0;
  int egl_minor = 0;
  EGLDisplay egl_display = eglGetDisplay(display);
  int egl_version_ok = eglInitialize(egl_display, &egl_major, &egl_minor);
  int egl_error = eglGetError();
  printf(
      "egl display %p egl version %d egl error 0x%x [ major %d ] [ minor %d "
      "]\n",
      egl_display, egl_version_ok, egl_error, egl_major, egl_minor);
  if (!egl_version_ok) {
    puts("fail eglInitialize");
    exit(1);
  }

  // egl config
  int egl_config_count;
  if (!eglGetConfigs(egl_display, NULL, 0, &egl_config_count)) {
    puts("fail eglGetConfigs");
    exit(1);
  }

#define MAX_EGL_CONFIG 40
  if (egl_config_count > MAX_EGL_CONFIG) {
    printf("warn egl_config_count %d exceeds fixed buffer %d", egl_config_count,
           MAX_EGL_CONFIG);
    egl_config_count = MAX_EGL_CONFIG;
  }

  EGLConfig egl_config[MAX_EGL_CONFIG];
  if (!eglGetConfigs(egl_display, egl_config, egl_config_count,
                     &egl_config_count)) {
    puts("fail eglGetConfigs");
    exit(1);
  }

  for (int i = 0; i < egl_config_count; ++i) {
    int color_type;
    int renderable_type;
#define EGL_READ_CONFIG(attrib, x) \
  eglGetConfigAttrib(egl_display, egl_config[i], attrib, x);
    EGL_READ_CONFIG(EGL_COLOR_BUFFER_TYPE, &color_type);
    EGL_READ_CONFIG(EGL_RENDERABLE_TYPE, &renderable_type);

    int red_bits;
    int green_bits;
    int blue_bits;
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_RED_SIZE, &red_bits);
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_GREEN_SIZE, &green_bits);
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_BLUE_SIZE, &blue_bits);

    int alpha_bits;
    int depth_bits;
    int stencil_bits;
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_ALPHA_SIZE, &alpha_bits);
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_DEPTH_SIZE, &depth_bits);
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_STENCIL_SIZE,
                       &stencil_bits);

    int egl_samples;
    eglGetConfigAttrib(egl_display, egl_config[i], EGL_SAMPLES, &egl_samples);

    printf(
        "[ color %x ] [ renderable %x ] [ r g b %d %d %d ] [ a d s %d %d %d ] "
        "[ samples %d ]\n",
        color_type, renderable_type, red_bits, green_bits, blue_bits,
        alpha_bits, depth_bits, stencil_bits, egl_samples);
  }

  // TODO: egl context
  if (!eglBindAPI(EGL_OPENGL_API)) {
    printf("fail eglBindAPI OPENGL: %d", eglGetError());
    exit(1);
  }
#define MAX_EGL_ATTRIB 16
  int attrib[MAX_EGL_ATTRIB] = {0};
  int* write_attrib = attrib;
  *write_attrib++ = EGL_CONTEXT_MAJOR_VERSION_KHR;
  *write_attrib++ = 3;
  *write_attrib++ = EGL_CONTEXT_MINOR_VERSION_KHR;
  *write_attrib++ = 3;
  *write_attrib++ = EGL_CONTEXT_FLAGS_KHR;
  *write_attrib++ = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
  *write_attrib++ = EGL_NONE;
  *write_attrib++ = EGL_NONE;

  EGLContext egl_context =
      eglCreateContext(egl_display, egl_config[1], NULL, attrib);
  if (!egl_context) {
    printf("fail eglCreateContext: 0x%x", eglGetError());
    exit(1);
  }

  // X11 window create
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

  XMapWindow(display, window_id);

  attrib[0] = EGL_NONE;
  attrib[1] = EGL_NONE;
  EGLSurface egl_surface =
      eglCreateWindowSurface(egl_display, egl_config[1], window_id, attrib);
  if (!egl_surface) {
    puts("fail eglCreateWindowSurface");
    exit(1);
  }

  XSetErrorHandler(x11_error_handler);
  XSetIOErrorHandler(x11_ioerror_handler);
  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

  while (running) {
    usleep(100);
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    eglSwapBuffers(egl_display, egl_surface);

    XEvent xev;
    while (XCheckWindowEvent(display, window_id, -1, &xev)) {
      printf("Event %d\n", xev.type);

      KeySym k;
      switch (xev.type) {
        case KeyPress: {
          k = XLookupKeysym(&xev.xkey, 0);
          printf("KeyPress 0x%lx\n", k);
          break;
        }
      }
    }
  }
  puts("goodbye");

  XUnmapWindow(display, window_id);
  XDestroyWindow(display, window_id);
  XCloseDisplay(display);
}
