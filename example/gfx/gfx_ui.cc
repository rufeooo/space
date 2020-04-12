#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"

void
RenderStringWithBoundingBox(const char* msg, v2f pos, v2f dims)
{
  int msg_len = strlen(msg);
  imui::TextOptions options;
  options.color = v4f(0.35f, 0.12f, 0.70f, 1.f);
  imui::Text(msg, pos, options);
  rgg::ModifyObserver mod(
      math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f), math::Identity());
  rgg::RenderLineRectangle(rgg::GetTextRect(msg, msg_len, pos),
                           v4f(1.0f, 0.0f, 0.0f, 1.0f));
}

void
TextTest()
{
  auto dims = window::GetWindowSize();

  imui::PaneOptions pane_options;
  imui::Begin(
      v2f(dims.x / 2.f - 400.0f, dims.y / 2.f + 160.0f), 0, pane_options);
  imui::TextOptions text_options;
  text_options.highlight_color = v4f(1.0f, 0.0f, 0.0f, 1.0f);
  text_options.color = gfx::kWhite;
  static bool is_clicked = false;
  if (imui::Text("Debug", text_options).highlighted) {
    imui::Indent(5);
    imui::Text("1.32241 + 32569 = NUMBER");
    imui::Text("I kInd. OF, h4te_ font");
    imui::Text("The quick brown fox");
    imui::Text("quick");
    imui::Text("To TL td tj Tj Pj pj PJ");
    imui::Text("1023");
    imui::Indent(-5);
  }
  imui::Text("Test");
  imui::End();

}

int
main(int argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  gfx::Initialize(create_info);


  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseClick(event.position, event.button, 0);
          }
        } break;
        default: break;
      }
    }

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextTest();
    imui::Render(0);
    imui::ResetAll();
    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

