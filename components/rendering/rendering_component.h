#pragma once

struct RenderingComponent {
  RenderingComponent() = default;
  RenderingComponent(
      uint32_t vao_reference, uint32_t program_reference,
      uint32_t vertex_count) :
    vao_reference(vao_reference), program_reference(program_reference),
    vertex_count(vertex_count) {}

  // ID to the vertices bound to OpenGL.
  uint32_t vao_reference;
  // ID to the compiled / linked OpenGL shader program.
  uint32_t program_reference;
  // Number of vertices to render for this component.
  uint32_t vertex_count;
};
