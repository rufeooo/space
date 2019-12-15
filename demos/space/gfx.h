#pragma once

// Do our best to keep all graphics related calls in here.

namespace gfx {

bool Initialize();

// Poll OS specific events.
void PollEvents();

bool Render();


}
