#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"
#include "../common/queue.cc"

EXTERN(unsigned imui_errno);

namespace imui
{
constexpr int kMaxTextSize = 128;
constexpr int kClickForFrames = 100;

static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.f, 0.f, 0.f, 0.5f);

struct Result {
  math::Rectf rect;
  bool highlighted = false;
  bool clicked = false;
};

struct TextOptions {
  v4f color = kWhite;
  v4f highlight_color = v4f();
  float scale = 1.0f;
};

struct PaneOptions {
  enum SizeMode {
    kAutoResize,
    kFixedSize,
  };
  PaneOptions() = default;
  PaneOptions(float width, float height)
      : size_mode(kFixedSize), width(width), height(height)
  {
  }

  SizeMode size_mode = kAutoResize;
  float width = 0.f;
  float height = 0.f;
};

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
  v4f color;  // TODO: This is duplicated in TextOptions
  math::Rectf rect;
  TextOptions options;
};

struct Pane {
  math::Rectf rect;
  PaneOptions options;
};

struct UIClick {
  v2f pos;
  PlatformButton button;
};

struct UIClickRender {
  v2f pos;
  int render_frames;
};

struct Button {
  math::Rectf rect;
  v4f color;
};

struct BeginMode {
  v2f pos;
  bool set = false;
  int text_calls = 0;
  uint32_t tag = 0;
  Pane* pane;
};

struct IMUI {
  BeginMode begin_mode;
};

struct Box {
  math::Rectf rect;
  v4f color;
  v4f outline_color;
};

static IMUI kIMUI;

constexpr uint32_t kMaxTags = 2;

DECLARE_ND_ARRAY(Text, kMaxTags, 64);
DECLARE_ND_ARRAY(Button, kMaxTags, 16);
DECLARE_ND_ARRAY(UIClick, kMaxTags, 8);
DECLARE_ND_ARRAY(Pane, kMaxTags, 8);
DECLARE_QUEUE(UIClickRender, 8);

void
Reset()
{
  ResetText();
  ResetButton();
  ResetPane();
  ResetUIClick();
}

void
Render(uint32_t tag)
{
  glDisable(GL_DEPTH_TEST);
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  for (int i = 0; i < kUsedPane[tag]; ++i) {
    Pane* pane = &kPane[tag][i];
    rgg::RenderRectangle(pane->rect, kPaneColor);
  }

  for (int i = 0; i < kUsedButton[tag]; ++i) {
    Button* button = &kButton[tag][i];
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedText[tag]; ++i) {
    Text* text = &kText[tag][i];
    rgg::RenderText(text->msg, text->pos, text->options.scale, text->color);
  }

  for (int i = kReadUIClickRender; i < kWriteUIClickRender; ++i) {
    UIClickRender* render = &kUIClickRender[i % kMaxUIClickRender];
    rgg::RenderCircle(
        render->pos, 3.5f,
        v4f(1.f, 0.f, 0.65f, (float)render->render_frames / kClickForFrames));
    --render->render_frames;
    if (!render->render_frames) {
      PopUIClickRender();
    }
  }
  glEnable(GL_DEPTH_TEST);
}

bool
IsRectHighlighted(math::Rectf rect)
{
  return math::PointInRect(window::GetCursorPosition(), rect);
}

bool
IsRectClicked(math::Rectf rect)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedUIClick[tag]; ++i) {
    UIClick* click = &kUIClick[tag][i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

void
Indent(int spaces)
{
  auto& font = rgg::kUI.font;
  FntMetadataRow* row = &font.metadata.rows[' '];
  if (!row || !row->id) return;
  kIMUI.begin_mode.pos.x += spaces * row->xadvance;
}

void
UpdatePaneOnCall(const math::Rectf& rect, Pane* pane)
{
  if (!pane) return;
  auto& begin_mode = kIMUI.begin_mode;
  // Must call Begin() before UI rendering starts and End() when it's done.
  assert(begin_mode.set);
  switch (pane->options.size_mode) {
    case PaneOptions::kAutoResize: {
      begin_mode.pane->rect.y = begin_mode.pos.y;
      begin_mode.pane->rect.height += rect.height;
      float width = (rect.x + rect.width) - begin_mode.pane->rect.x;
      if (width > begin_mode.pane->rect.width) {
        begin_mode.pane->rect.width = width;
      }
    } break;
    case PaneOptions::kFixedSize:
      break;
    default:
      break;
  }
}

void
UpdatePaneOnEnd(Pane* pane)
{
  if (!pane) return;
  auto& begin_mode = kIMUI.begin_mode;
  uint32_t tag = begin_mode.tag;
  assert(begin_mode.set);
  switch (pane->options.size_mode) {
    case PaneOptions::kAutoResize:
      break;
    case PaneOptions::kFixedSize: {
      int start = kUsedText[tag] - 1;
      int end = kUsedText[tag] - begin_mode.text_calls;
      assert(start < kMaxText);
      assert(end >= 0);
      int move_text = begin_mode.text_calls - 1;
      for (; start >= end; --start) {
        struct Text* text = &kText[tag][start];
        text->pos.y += text->rect.height * move_text;
        v2f text_top_left = text->pos + v2f(0.f, text->rect.height);
        // Discard the text element if it is outside of the pane.
        if (!math::PointInRect(text_top_left, pane->rect)) {
          CompressText(tag, start);
          continue;
        }
      }
    } break;
    default:
      break;
  }
}

Result
Text(const char* msg, v2f pos, TextOptions options)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  Result data;
  struct Text* text = UseText(tag);
  if (!text) {
    imui_errno = 1;
    return data;
  }
  if (strlen(msg) > kMaxTextSize) {
    imui_errno = 2;
    return data;
  }
  auto& begin_mode = kIMUI.begin_mode;
  data.rect = rgg::GetTextRect(msg, strlen(msg), begin_mode.pos, options.scale);
  data.highlighted = IsRectHighlighted(data.rect);
  data.clicked = IsRectClicked(data.rect);
  strcpy(text->msg, msg);
  text->pos = pos;
  text->color = options.color;
  if (data.highlighted && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  text->options = options;
  text->rect = data.rect;
  ++begin_mode.text_calls;
  return data;
}

// TODO(abrunasso): Consider removing. I think only allowing UI calls between
// Consider making all UI calls occur between Begin / End. Otherwise there
// will be many code paths for UI elements and state becomes hard to reason
// about.
Result
Text(const char* msg, v2f pos)
{
  return Text(msg, pos, {kWhite, kWhite});
}

Result
Text(const char* msg, TextOptions options)
{
  auto& begin_mode = kIMUI.begin_mode;
  // Call StartText before this.
  assert(kIMUI.begin_mode.set);
  Result data = Text(msg, begin_mode.pos, options);
  UpdatePaneOnCall(data.rect, begin_mode.pane);
  begin_mode.pos.y -= data.rect.height;
  return data;
}

Result
Text(const char* msg)
{
  return Text(msg, {kWhite, kWhite});
}

void
Begin(v2f start, uint32_t tag, const PaneOptions& pane_options)
{
  assert(tag < kMaxTags);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = start;
  begin_mode.set = true;
  begin_mode.text_calls = 0;
  begin_mode.tag = tag;
  begin_mode.pane = UsePane(tag);
  begin_mode.pane->rect.x = start.x;
  begin_mode.pane->rect.y = start.y;
  begin_mode.pane->rect.width = pane_options.width;
  begin_mode.pane->rect.height = pane_options.height;
  begin_mode.pane->options = pane_options;
}

void
Begin(v2f start, uint32_t tag)
{
  assert(tag < kMaxTags);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = start;
  begin_mode.set = true;
  begin_mode.tag = tag;
  begin_mode.pane = nullptr;
}

void
End()
{
  UpdatePaneOnEnd(kIMUI.begin_mode.pane);
  kIMUI.begin_mode.set = false;
}

Result
Button(const math::Rectf& rect, const v4f& color)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  struct Button* button = UseButton(tag);
  Result result;
  if (!button) {
    imui_errno = 3;
    return result;
  }
  button->rect = rect;
  button->color = color;
  result.clicked = IsRectClicked(rect);
  result.highlighted = IsRectHighlighted(rect);
  result.rect = rect;
  return result;
}

void
MouseClick(v2f pos, PlatformButton b, uint32_t tag)
{
  UIClick* click = UseUIClick(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
  // PushUIClickRender({pos, kClickForFrames});
}

const char*
LastErrorString()
{
  unsigned err = imui_errno;
  imui_errno = 0;
  switch (err) {
    case 1:
      return ("imui text count exhausted.");
    case 2:
      return ("text provided surpasses max allowed imui character count.");
    case 3:
      return ("imui button count exhausted.");
    case 4:
      return ("imui click count exhausted.");
  };

  return 0;
}

}  // namespace imui
