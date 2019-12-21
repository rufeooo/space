#include "window.h"

#include <cassert>
#include <windows.h>
#include <gl/gl.h>
#include <stdbool.h>

namespace window {

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
        const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

struct Window {
  // Handle to the current window.
  HWND hwnd;
  // Handle to the window device context.
  HDC hdc;
  // This is a handle to the OpenGL rendering context.
  HGLRC hglrc;
  // Whether the window should be closing or not.
  bool should_close = false;
  // Bitfield where each bit represents whether a key is down or not.
  // a bit value of 1 represents a key is pressed and 0 is down
  uint32_t input_mask;
  // Input mask from the previous frame.
  uint32_t previous_input_mask;
};

static Window kWindow;

void
HandleKedownEvent(WPARAM wparam)
{
  // Capture key events here.
  switch (wparam) {
    case 'W':
      kWindow.input_mask |= (1 << KEY_W);
      break;
    case 'A':
      kWindow.input_mask |= (1 << KEY_A);
      break;
    case 'S':
      kWindow.input_mask |= (1 << KEY_S);
      break;
    case 'D':
      kWindow.input_mask |= (1 << KEY_D);
      break;
    default:
      break;
  }
}

LRESULT CALLBACK
WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
  LRESULT result = 0;

  switch (msg) {
    case WM_CLOSE:
    case WM_DESTROY: {
      PostQuitMessage(0);
    } break;
    case WM_KEYDOWN: {
      HandleKedownEvent(wparam);
    } break;
    case WM_LBUTTONDOWN: {
      kWindow.input_mask |= (1 << MOUSE_LEFT_CLICK);
    } break;
    default: {
      result = DefWindowProcA(window, msg, wparam, lparam); 
    } break;
  }

  return result;
}

HWND
SetupWindow(HINSTANCE inst, const char* name, int width, int height)
{
  WNDCLASSA wc = {};
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = inst;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = 0;
  wc.lpszClassName = name;

  if (!RegisterClassA(&wc)) {
    assert("Failed to register window.");
  }

  RECT rect = {};
  rect.right = width;
  rect.bottom = height;

  DWORD window_style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRect(&rect, window_style, false);

  HWND window = CreateWindowExA(
      0,
      wc.lpszClassName,
      name,
      window_style,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      rect.right - rect.left,
      rect.bottom - rect.top,
      0,
      0,
      inst,
      0);

  if (!window) {
    assert("Failed to create window.");
  }

  return window;
}

void
InitOpenGLExtensions(void)
{
  // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
  // We use a dummy window because you can only set the pixel format for a window once. For the
  // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
  // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
  // have a context.
  WNDCLASSA wc = {};
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = DefWindowProcA;
  wc.hInstance = GetModuleHandle(0);
  wc.lpszClassName = "Dummy_WGL_djuasiodwa";

  if (!RegisterClassA(&wc)) {
    assert("Failed to register dummy OpenGL window.");
  }

  HWND dummy_window = CreateWindowExA(
      0,
      wc.lpszClassName,
      "NULL WINDOW",
      0,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      wc.hInstance,
      0);

  if (!dummy_window) {
    assert("Failed to create dummy OpenGL window.");
  }

  HDC dummy_dc = GetDC(dummy_window);

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cAlphaBits = 8;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
  if (!pixel_format) {
    assert("Failed to find a suitable pixel format.");
  }
  if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
    assert("Failed to set the pixel format.");
  }

  HGLRC dummy_context = wglCreateContext(dummy_dc);
  if (!dummy_context) {
    assert("Failed to create a dummy OpenGL rendering context.");
  }

  if (!wglMakeCurrent(dummy_dc, dummy_context)) {
    assert("Failed to activate dummy OpenGL rendering context.");
  }

  wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
      "wglCreateContextAttribsARB");
  assert(wglCreateContextAttribsARB);
  wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
      "wglChoosePixelFormatARB");
  assert(wglChoosePixelFormatARB);

  wglMakeCurrent(dummy_dc, 0);
  wglDeleteContext(dummy_context);
  ReleaseDC(dummy_window, dummy_dc);
  DestroyWindow(dummy_window);
}

HGLRC
InitOpenGL(HDC real_dc)
{
  InitOpenGLExtensions();

  int pixel_format_attribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
      WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
      WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB,     32,
      WGL_DEPTH_BITS_ARB,     24,
      WGL_STENCIL_BITS_ARB,    8,
      0
  };

  int pixel_format;
  UINT num_formats;
  wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
  if (!num_formats) {
    assert("Failed to set the OpenGL 3.3 pixel format.");
  }

  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
    assert("Failed to set the OpenGL 3.3 pixel format.");
  }

  // Specify that we want to create an OpenGL 3.3 core profile context
  int gl33_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0,
  };

  HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
  if (!gl33_context) {
    assert("Failed to create OpenGL 3.3 context.");
  }

  if (!wglMakeCurrent(real_dc, gl33_context)) {
    assert("Failed to activate OpenGL 3.3 rendering context.");
  }

  return gl33_context;
}

void
Create(const char* name, int width, int height)
{
  kWindow.hwnd = SetupWindow(GetModuleHandle(0), name, width, height);
  kWindow.hdc = GetDC(kWindow.hwnd);
  kWindow.hglrc = InitOpenGL(kWindow.hdc);

  ShowWindow(kWindow.hwnd, 1);
  UpdateWindow(kWindow.hwnd);
}

void
PollEvents()
{
  kWindow.previous_input_mask = kWindow.input_mask;
  kWindow.input_mask = 0;
  MSG msg;
  while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      kWindow.should_close = true;
    } else {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }
  }
}

void
SwapBuffers()
{
  SwapBuffers(kWindow.hdc);
}

bool
ShouldClose()
{
  return kWindow.should_close;
}

math::Vec2f
GetWindowSize()
{
  RECT rect;
  GetWindowRect(kWindow.hwnd, &rect);
  return math::Vec2f((float)rect.right - rect.left, (float)rect.bottom - rect.top);
}


math::Vec2f
GetCursorPosition()
{
  POINT cursor;
  GetCursorPos(&cursor);
  ScreenToClient(kWindow.hwnd, &cursor);
  return math::Vec2f((float)cursor.x, (float)cursor.y);
}

bool
HasInput(Input key)
{
  return (kWindow.input_mask & (1 << key)) != 0;
}
}  // namespace window
