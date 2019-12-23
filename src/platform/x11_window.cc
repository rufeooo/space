#define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "window.h"

static int last_window_error;
Display* display;
int window_id;

namespace window
{
int
x11_error_handler(Display*, XErrorEvent*)
{
  return 0;
}

int
x11_ioerror_handler(Display*)
{
  return 0;
}

void
Create(const char*, int width, int height)
{
  if (window_id) return;

  display = XOpenDisplay(NULL);
  int screen = DefaultScreen(display);
  Window root_window = RootWindow(display, screen);

  int egl_major = 0;
  int egl_minor = 0;
  EGLDisplay egl_display = eglGetDisplay(display);
  int egl_version_ok = eglInitialize(egl_display, &egl_major, &egl_minor);
  if (!egl_version_ok) {
    return;
  }

  // egl config
  int egl_config_count;
  if (!eglGetConfigs(egl_display, NULL, 0, &egl_config_count)) {
    return;
  }

#define MAX_EGL_CONFIG 40
  if (egl_config_count > MAX_EGL_CONFIG) {
    egl_config_count = MAX_EGL_CONFIG;
  }

  EGLConfig egl_config[MAX_EGL_CONFIG];
  if (!eglGetConfigs(egl_display, egl_config, egl_config_count,
                     &egl_config_count)) {
    return;
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
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    return;
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
    return;
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
  Window parent = root_window;
  window_id = XCreateWindow(display, parent, x, y, width, height, border_width,
                            depth, InputOutput, CopyFromParent, wamask, &wa);

  if (!window_id) {
    return;
  }

  XMapWindow(display, window_id);

  attrib[0] = EGL_NONE;
  attrib[1] = EGL_NONE;
  EGLSurface egl_surface =
      eglCreateWindowSurface(egl_display, egl_config[1], window_id, attrib);
  if (!egl_surface) {
    return;
  }

  XSetErrorHandler(x11_error_handler);
  XSetIOErrorHandler(x11_ioerror_handler);
  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
}

void
Destroy()
{
  XUnmapWindow(display, window_id);
  XDestroyWindow(display, window_id);
  XCloseDisplay(display);
}

void
SwapBuffers()
{
}

bool
PollEvent(Event* event)
{
  event->type = NOT_IMPLEMENTED;
  event->key = 0;
  event->position = math::Vec2f(0.f, 0.f);

  XEvent xev;
  while (XCheckWindowEvent(display, window_id, -1, &xev)) {
    switch (xev.type) {
    }

    return true;
  }

  return false;
}

}  // namespace window
