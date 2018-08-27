#ifndef ENTIS_COLOR_H_
#define ENTIS_COLOR_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <inttypes.h>

typedef struct entis_color {
  uint8_t red, green, blue;
} entis_color;

typedef struct entis_palette {
  struct entis_color colors[16];
} entis_palette;

struct entis_color entis_create_color(uint8_t red, uint8_t green, uint8_t blue);
struct entis_color entis_create_color_int(uint32_t rgb);
struct entis_color entis_create_color_double(double red, double green,
                                             double blue);

uint32_t entis_get_color_value(struct entis_color color);

struct entis_palette entis_create_default_palette();
struct entis_palette entis_create_palette(uint8_t colors[16][3]);
struct entis_palette entis_create_palette_int(uint32_t colors[16]);
struct entis_palette entis_create_palette_double(double colors[16][3]);
struct entis_palette entis_create_palette_struct(struct entis_color colors[16]);
void entis_set_palette_color(struct entis_palette* palette, int index,
                             uint8_t red, uint8_t green, uint8_t blue);
void entis_set_palette_color_int(struct entis_palette* palette, int index,
                             uint32_t rgb);
void entis_set_palette_color_double(struct entis_palette* palette, int index,
                                    double red, double green, double blue);
void entis_set_palette_color_struct(struct entis_palette* palette, int index,
                                    struct entis_color color);

struct entis_color entis_palette_get_color(struct entis_palette palette,
                                           int index);

#ifdef __cplusplus
}
#endif

#endif  // ENTIS_COLOR_H_
