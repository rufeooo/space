#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"

namespace imui
{
constexpr int kMaxTextSize = 128;

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
};

struct UIClick {
  v2f pos;
};

struct Button {
  math::Rect rect;
  v4f color;
};

struct TextMode {
  v2f pos;
  bool set;
};

struct IMUI {
  TextMode text_mode;
};

static IMUI kIMUI;

DECLARE_ARRAY(Text, 32);
DECLARE_ARRAY(Button, 16);
DECLARE_ARRAY(UIClick, 8);

void
Reset()
{
  kUsedText = 0;
  kUsedButton = 0;
  kUsedUIClick = 0;
}

void
Render()
{
  for (int i = 0; i < kUsedButton; ++i) {
    Button* button = &kButton[i];
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedText; ++i) {
    Text* text = &kText[i];
    rgg::RenderText(text->msg, text->pos, v4f());
  }
}

void
Text(const char* msg, v2f pos)
{
  struct Text* text = UseText();
  if (!text) {
    printf("imui text count exhausted.\n");
    return;
  }
  if (strlen(msg) > kMaxTextSize) {
    printf("text provided surpasses max allowed imui character count.\n");
    return;
  }
  strcpy(text->msg, msg);
  text->pos = pos;
}

void
BeginText(v2f start)
{
  auto& text_mode = kIMUI.text_mode;
  text_mode.pos = start;
  text_mode.set = true; 
}

void
Text(const char* msg)
{
  auto& text_mode = kIMUI.text_mode;
  // Call StartText before this.
  assert(kIMUI.text_mode.set);
  Text(msg, text_mode.pos);
  math::Rect rect = rgg::GetTextRect(msg, strlen(msg), text_mode.pos);
  text_mode.pos.y -= rect.height;
}

void
EndText()
{
  kIMUI.text_mode.set = true; 
}

bool
IsButtonClicked(const math::Rect& rect)
{
  for (int i = 0; i < kUsedUIClick; ++i) {
    UIClick* click = &kUIClick[i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

bool
Button(const math::Rect& rect, const v4f& color)
{
  struct Button* button = UseButton();
  if (!button) {
    printf("imui button count exhausted.\n");
    return false;
  }
  button->rect = rect;
  button->color = color;
  return IsButtonClicked(rect);
}

void
MouseClick(v2f pos)
{
  UIClick* click = UseUIClick();
  if (!click) {
    printf("imui click count exhausted.\n");
    return;
  }
  click->pos = pos;
}

}
