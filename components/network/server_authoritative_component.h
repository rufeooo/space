#pragma once

namespace component {

template <class T>
struct ServerAuthoratativeComponent {
  // Only update components if the previous sample does not match
  // the current sample.
  T current_sample;
  T previous_sample;
};

}
