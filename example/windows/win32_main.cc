#include <windows.h>

LRESULT CALLBACK
WindowProc(
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
  switch (uMsg) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    } break;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      // All painting occurs here, between BeginPaint and EndPaint.

      FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

      EndPaint(hwnd, &ps);
    } break;


    case WM_CHAR: {
      switch (wParam) {
      case 27:			/* ESC key */
          PostQuitMessage(0);
          break;
      }
    } break;

  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        PSTR lpCmdLine, INT nCmdShow)
{
  // Register the window class.
  const wchar_t CLASS_NAME[]  = L"Windows is great!";

  WNDCLASS wc = { };

  wc.lpfnWndProc   = WindowProc;
  wc.hInstance     = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&wc);

  HWND hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    L"Learn to Program Windows",    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

    NULL,       // Parent window    
    NULL,       // Menu
    hInstance,  // Instance handle
    NULL        // Additional application data
  );

  ShowWindow(hwnd, nCmdShow);

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
