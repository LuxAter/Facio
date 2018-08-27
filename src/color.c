#include <inttypes.h>

#include "color.h"

struct entis_color entis_create_color(uint8_t red, uint8_t green,
                                      uint8_t blue) {
  struct entis_color color = {red, green, blue};
  return color;
}
struct entis_color entis_create_color_int(uint32_t rgb) {
  struct entis_color color;
  color.blue = rgb % 256;
  rgb = (rgb - color.blue) / 256;
  ;
  color.green = rgb % 256;
  rgb = (rgb - color.green) / 256;
  ;
  color.red = rgb % 256;
  return color;
}
struct entis_color entis_create_color_double(double red, double green,
                                             double blue) {
  struct entis_color color;
  color.red = (uint8_t)(255 * red);
  color.green = (uint8_t)(255 * green);
  color.blue = (uint8_t)(255 * blue);
  return color;
}

uint32_t entis_get_color_value(struct entis_color color) {
  uint32_t result =
      (256 * 256 * color.red) + (256 * color.green) + (color.blue);
  return result;
}

struct entis_palette entis_create_default_palette() {
  struct entis_palette palette;
  palette.colors[0] = entis_create_color(000, 000, 000);
  palette.colors[1] = entis_create_color(128, 000, 000);
  palette.colors[2] = entis_create_color(000, 128, 000);
  palette.colors[3] = entis_create_color(128, 128, 000);
  palette.colors[4] = entis_create_color(000, 000, 128);
  palette.colors[5] = entis_create_color(128, 000, 128);
  palette.colors[6] = entis_create_color(000, 128, 128);
  palette.colors[7] = entis_create_color(192, 192, 192);
  palette.colors[8] = entis_create_color(128, 128, 128);
  palette.colors[9] = entis_create_color(255, 000, 000);
  palette.colors[10] = entis_create_color(000, 255, 000);
  palette.colors[11] = entis_create_color(255, 255, 000);
  palette.colors[12] = entis_create_color(000, 000, 255);
  palette.colors[13] = entis_create_color(255, 000, 255);
  palette.colors[14] = entis_create_color(000, 255, 255);
  palette.colors[15] = entis_create_color(255, 255, 255);
  return palette;
}

struct entis_palette entis_create_palette(uint8_t colors[16][3]) {
  struct entis_palette palette;
  for (int i = 0; i < 16; i++) {
    palette.colors[i] =
        entis_create_color(colors[i][0], colors[i][1], colors[i][2]);
  }
  return palette;
}

struct entis_palette entis_create_palette_int(uint32_t colors[16]) {
  struct entis_palette palette;
  for (int i = 0; i < 16; i++) {
    palette.colors[i] = entis_create_color_int(colors[i]);
  }
  return palette;
}

struct entis_palette entis_create_palette_double(double colors[16][3]) {
  struct entis_palette palette;
  for (int i = 0; i < 16; i++) {
    palette.colors[i] =
        entis_create_color_double(colors[i][0], colors[i][1], colors[i][2]);
  }
  return palette;
}

struct entis_palette entis_create_palette_struct(
    struct entis_color colors[16]) {
  struct entis_palette palette;
  for (int i = 0; i < 16; i++) {
    palette.colors[i] = colors[i];
  }
  return palette;
}
void entis_set_palette_color(struct entis_palette* palette, int index,
                             uint8_t red, uint8_t green, uint8_t blue) {
  palette->colors[index] = entis_create_color(red, green, blue);
}
void entis_set_palette_color_int(struct entis_palette* palette, int index,
                             uint32_t rgb) {
  palette->colors[index] = entis_create_color_int(rgb);
}
void entis_set_palette_color_double(struct entis_palette* palette, int index,
                                    double red, double green, double blue) {
  palette->colors[index] = entis_create_color_double(red, green, blue);
}
void entis_set_palette_color_struct(struct entis_palette* palette, int index,
                                    struct entis_color color) {
  palette->colors[index] = color;
}

struct entis_color entis_palette_get_color(struct entis_palette palette,
                                           int index) {
  return palette.colors[index];
}
