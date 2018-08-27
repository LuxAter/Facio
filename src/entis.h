#ifndef ENTIS_ENTIS_H_
#define ENTIS_ENTIS_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <xcb/xcb.h>

#include "event.h"
#include "key_codes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ENTIS_D30 0.52359877559829881565
#define ENTIS_D45 0.78539816339744827899
#define ENTIS_D60 1.04719755119659763131
#define ENTIS_D90 1.57079632679489655799

#define ENTIS_BLACK 0x000000
#define ENTIS_RED 0xFF0000
#define ENTIS_GREEN 0x00FF00
#define ENTIS_BLUE 0x0000FF
#define ENTIS_YELLOW 0xFFFF00
#define ENTIS_MAGENTA 0xFF00FF
#define ENTIS_CYAN 0x00FFFF
#define ENTIS_WHITE 0xFFFFFF
#define ENTIS_DARK_RED 0x800000
#define ENTIS_DARK_GREEN 0x008000
#define ENTIS_DARK_BLUE 0x000080
#define ENTIS_DARK_YELLOW 0x808000
#define ENTIS_DARK_MAGENTA 0x800080
#define ENTIS_DARK_CYAN 0x008080
#define ENTIS_DARK_WHITE 0x808080

/**
 * \defgroup General
 * General functions for support of the Entis library.
 * @{ */

/**
 * @brief Initialize Entis library.
 *
 * This **must** be called before all other Entis calls. It generates the XCB
 * connection to the X server, which is necessary for almost all functions
 * present in the Entis library. Thus be sure to call this function before any
 * other functions from Entis. It also generate a window with provided
 * specifics.
 *
 * @param title Title of the window to generate.
 * @param w Desired width of the window.
 * @param h Desired height of the window.
 * @param value_mask Bit mask of values to alter the window.
 * @param value_list Values associated with provided bits in `value_mask`.
 */
void entis_init(const char* title, unsigned int w, unsigned int h,
                uint32_t value_mask, void* value_list);
/**
 * @brief Terminates Entis library.
 *
 * This function **must** be the final Entis function called. After it is
 * called no other Entis function will work properly. It closes the XCB
 * connection to the X server, which is necessary for almost all functions.
 * This function will also close the generated window.
 */
void entis_term();

/**
 * @brief Checks if XCB connection is valid
 *
 * Checks if the currently established XCB connection to the X server is valid.
 *
 * @return `true` or `false`.
 */
bool entis_connection_valid();
/**
 * @brief Flush XCB connections
 *
 * Sends all requests to the X server. This must be done for any requests to be
 * acted upon.
 */
void entis_flush();

/**  @} */

xcb_connection_t* entis_get_connection();
xcb_screen_t* entis_get_screen();
xcb_window_t entis_get_window();
xcb_pixmap_t entis_get_pixmap();
xcb_gcontext_t entis_get_gcontext();

void entis_set_color(uint32_t color);
void entis_set_color_rgb(uint32_t r, uint32_t g, uint32_t b);
void entis_set_color_drgb(double r, double g, double b);
void entis_set_background(uint32_t color);
void entis_set_background_rgb(uint32_t r, uint32_t g, uint32_t b);
void entis_set_background_drgb(double r, double g, double b);

void entis_reload_pixmap(uint32_t w, uint32_t h);

void entis_copy_pixmap();
void entis_update();
void entis_clear();

void entis_sleep(double sec);

entis_event entis_wait_event();
entis_event entis_poll_event();

entis_event entis_wait_event_type(uint32_t type);
entis_event entis_poll_event_type(uint32_t type);

entis_key_event entis_wait_key();
entis_key_event entis_poll_key();

entis_button_event entis_wait_button();
entis_button_event entis_poll_button();

void entis_clear_events();

void entis_point(uint16_t x, uint16_t y);
void entis_segment(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void entis_line(uint16_t* x, uint16_t* y, uint16_t n);
void entis_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2);
void entis_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void entis_arc(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
               uint16_t angle1, uint16_t angle2);
void entis_poly(uint16_t* x, uint16_t* y, uint16_t n);

void entis_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                         uint16_t x2, uint16_t y2);
void entis_fill_rectangle(uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height);
void entis_fill_arc(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                    uint16_t angle1, uint16_t angle2);
void entis_fill_poly(uint16_t* x, uint16_t* y, uint16_t n);

void entis_reg_poly(uint16_t x, uint16_t y, uint16_t radius_x,
                    uint16_t radius_y, uint16_t n, double offset);
void entis_fill_reg_poly(uint16_t x, uint16_t y, uint16_t radius_x,
                         uint16_t radius_y, uint16_t n, double offset);

void entis_circle(uint16_t x, uint16_t y, uint16_t radius);
void entis_fill_circle(uint16_t x, uint16_t y, uint16_t radius);

uint16_t entis_get_pixel_width();
uint16_t entis_get_pixel_height();
void entis_set_pixel_size(uint16_t width, uint16_t height);
void entis_set_pixel(uint16_t x, uint16_t y);
void entis_pixel_set_pixel(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif  // ENTIS_ENTIS_H_
