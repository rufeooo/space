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
  uint8_t color_map_spec[5];
};
#pragma pack(pop)

uint32_t
LoadTGA(const char* file, uint8_t** image_bytes,
        uint16_t* image_width, uint16_t* image_height)
{
  FILE* fptr;
  uint8_t* buffer;
  uint32_t file_length;

  fptr = fopen(file, "rb");
  fseek(fptr, 0, SEEK_END);
  file_length = ftell(fptr);
  rewind(fptr);
  buffer = (uint8_t*)malloc(file_length);
  fread(buffer, file_length, 1, fptr);

  // First load the header.
  TgaHeader* header = (TgaHeader*)buffer;
  // Just don't even support colors.
  assert(header->id_length == 0);
  assert(header->color_map_type == 0);
  // Get the image_spec. This has overall image details. 
  TgaImageSpec* image_spec = (TgaImageSpec*)(&buffer[sizeof(TgaHeader)]);
  // Only support 8-bit pixel depths.
  assert(image_spec->pixel_depth == 8);

  // Image bytes sz
  uint32_t image_bytes_size = image_spec->image_width * image_spec->image_height;
  *image_width = image_spec->image_width;
  *image_height = image_spec->image_height;
  *image_bytes = (uint8_t*)malloc(image_bytes_size);
  memcpy(*image_bytes, &buffer[sizeof(TgaHeader) + sizeof(TgaImageSpec)], image_bytes_size);
  // Free buffer used to read in file.
  free(buffer);
  return image_bytes_size;
}
/*
int
main(int argc, char** argv)
{
  uint8_t* image_bytes;
  
  uint32_t sz = LoadTGA("example/gfx/characters_0.tga", &image_bytes);

  printf("read %i image bytes...\n", sz);

  // First pixel is pure black.
  printf("pixel: %i\n", image_bytes[0]);

  // These should fade to white
  printf("pixel: %i\n", image_bytes[7]);
  printf("pixel: %i\n", image_bytes[8]);
  printf("pixel: %i\n", image_bytes[8]);
  printf("pixel: %i\n", image_bytes[10]);
  printf("pixel: %i\n", image_bytes[11]);
  printf("pixel: %i\n", image_bytes[12]);
  printf("pixel: %i\n", image_bytes[13]);
  printf("pixel: %i\n", image_bytes[14]);


  return 0;
}*/
