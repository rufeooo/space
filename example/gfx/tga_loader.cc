#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// http://tfc.duke.free.fr/coding/tga_specs.pdf

#pragma pack(push,1)
struct TgaImageSpec {
  uint16_t x_origin;
  uint16_t y_origin;
  uint16_t image_width;
  uint16_t image_height;
  uint8_t pixel_depth;
  uint8_t image_descriptor;
};
struct TgaHeader {
  uint8_t id_length;
  uint8_t color_map_type;
  uint8_t image_type;
  // Ignore this for now.
  uint8_t color_map_spec[5];
};
#pragma pack(pop)

int main(int argc, char** argv) {
  FILE* fptr;
  uint8_t* buffer;
  uint32_t file_length;

  fptr = fopen("example/gfx/characters_0.tga", "rb");
  fseek(fptr, 0, SEEK_END);
  file_length = ftell(fptr);
  rewind(fptr);
  buffer = (uint8_t*)malloc(file_length);
  fread(buffer, file_length, 1, fptr);

  // First load the header.
  TgaHeader* header = (TgaHeader*)buffer;
  printf("File size: %i\n", file_length);
  printf("id_length: %hhx\n", header->id_length);
  printf("color_map_type: %x\n", header->color_map_type);
  printf("image_type: %x\n", header->image_type);
  printf("color_map_spec: %.*s\n", 5, header->color_map_spec); // probably 0 if no color

  printf("%zi\n", sizeof(TgaHeader));
  // Get the image_spec. This has overall image details. 
  TgaImageSpec* image_spec = (TgaImageSpec*)(&buffer[sizeof(TgaHeader)]);
  printf("x_origin: %i\n", image_spec->x_origin);
  printf("y_origin: %i\n", image_spec->y_origin);
  printf("image_width: %i\n", image_spec->image_width);
  printf("image_height: %i\n", image_spec->image_height);
  printf("pixel_depth: %i\n", image_spec->pixel_depth);
  printf("image_descriptor: %i\n", image_spec->image_descriptor);

  // Just don't even support colors.
  assert(header->id_length == 0);
  assert(header->color_map_type == 0);
  assert(image_spec->pixel_depth == 8);

  // Image bytes sz
  uint32_t image_bytes_size = image_spec->image_width * image_spec->image_height;

  uint8_t* pixel_bytes = (uint8_t*)malloc(image_bytes_size);
  memcpy(pixel_bytes, &buffer[sizeof(TgaHeader) + sizeof(TgaImageSpec)], image_bytes_size);

  // First pixel is pure black.
  printf("pixel: %i\n", pixel_bytes[0]);

  // These should fade to white
  printf("pixel: %i\n", pixel_bytes[7]);
  printf("pixel: %i\n", pixel_bytes[8]);
  printf("pixel: %i\n", pixel_bytes[8]);
  printf("pixel: %i\n", pixel_bytes[10]);
  printf("pixel: %i\n", pixel_bytes[11]);
  printf("pixel: %i\n", pixel_bytes[12]);
  printf("pixel: %i\n", pixel_bytes[13]);
  printf("pixel: %i\n", pixel_bytes[14]);

  return 0;
}
