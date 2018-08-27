#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <xcb/xcb.h>

#include "entis.h"
#include "event.h"

static xcb_connection_t* connection_;
static xcb_screen_t* screen_;

static xcb_window_t window_;
static xcb_pixmap_t pixmap_;
static xcb_gcontext_t gcontext_, pixmap_gcontext_, pixmap_bg_gcontext_;

static uint16_t width_, height_;

static uint16_t pix_width_, pix_height_;

void entis_init(const char* title, unsigned int w, unsigned int h,
                uint32_t value_mask, void* value_list) {
  if (connection_ != NULL) {
    fprintf(stderr, "Connection to X server already established!\n");
  }
  connection_ = xcb_connect(NULL, NULL);
  if (connection_ == NULL) {
    fprintf(stderr, "Failed to connect to X server\n");
    exit(1);
  } else if (xcb_connection_has_error(connection_)) {
    fprintf(stderr, "Error while attempting to connect to X server\n");
    xcb_disconnect(connection_);
    exit(1);
  }
  screen_ = xcb_setup_roots_iterator(xcb_get_setup(connection_)).data;
  window_ = xcb_generate_id(connection_);
  xcb_void_cookie_t cookie;
  if (value_mask != 0) {
    cookie = xcb_create_window(connection_, XCB_COPY_FROM_PARENT, window_,
                               screen_->root, 0, 0, w, h, 0,
                               XCB_WINDOW_CLASS_INPUT_OUTPUT,
                               screen_->root_visual, value_mask, value_list);
  } else {
    uint32_t internal_value[1] = {
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY};
    cookie = xcb_create_window(
        connection_, XCB_COPY_FROM_PARENT, window_, screen_->root, 0, 0, w, h,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen_->root_visual,
        XCB_CW_EVENT_MASK, internal_value);
  }
  xcb_generic_error_t* error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to generage xcb window\n",
            error_check->error_code);
    xcb_destroy_window(connection_, window_);
    exit(1);
  }
  free(error_check);
  pixmap_ = xcb_generate_id(connection_);
  cookie = xcb_create_pixmap(connection_, screen_->root_depth, pixmap_,
                             screen_->root, w, h);
  error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to generate xcb pixmap\n",
            error_check->error_code);
  }
  free(error_check);
  pixmap_gcontext_ = xcb_generate_id(connection_);
  cookie = xcb_create_gc(connection_, pixmap_gcontext_, screen_->root, 0, NULL);
  error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr,
            "[ERROR %u] Failed to generage xcb pixmap graphics context\n",
            error_check->error_code);
  }
  free(error_check);
  pixmap_bg_gcontext_ = xcb_generate_id(connection_);
  cookie = xcb_create_gc(connection_, pixmap_bg_gcontext_, screen_->root,
                         XCB_GC_FOREGROUND, (uint32_t[]){0x000000});
  error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr,
            "[ERROR %u] Failed to generage xcb pixmap graphics context\n",
            error_check->error_code);
  }
  free(error_check);
  xcb_poly_fill_rectangle(connection_, pixmap_, pixmap_bg_gcontext_, 1,
                          (xcb_rectangle_t[]){{0, 0, w, h}});
  gcontext_ = xcb_generate_id(connection_);
  cookie = xcb_create_gc(connection_, gcontext_, screen_->root, 0, NULL);
  error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to generage xcb graphics context\n",
            error_check->error_code);
  }
  free(error_check);
  if (strlen(title) != 0) {
    xcb_change_property(connection_, XCB_PROP_MODE_REPLACE, window_,
                        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title),
                        title);
  }
  width_ = w;
  height_ = h;
  xcb_map_window(connection_, window_);
  xcb_flush(connection_);
  nanosleep(&(struct timespec){0, 3.125e7}, NULL);
  entis_clear_events();
}

void entis_term() {
  if (connection_ == NULL) {
    fprintf(stderr, "No connection to X server\n");
  }
  xcb_destroy_window(connection_, window_);
  xcb_disconnect(connection_);
}

bool entis_connection_valid() {
  if (connection_ == NULL) {
    return false;
  }
  return true;
}
void entis_flush() { xcb_flush(connection_); }

xcb_connection_t* entis_get_connection() { return connection_; }
xcb_screen_t* entis_get_screen() { return screen_; }
xcb_window_t entis_get_window() { return window_; }
xcb_pixmap_t entis_get_pixmap() { return pixmap_; }
xcb_gcontext_t entis_get_gcontext() { return gcontext_; }

void entis_set_color(uint32_t color) {
  uint32_t values[1] = {color};
  xcb_void_cookie_t cookie =
      xcb_change_gc(connection_, pixmap_gcontext_, XCB_GC_FOREGROUND, values);
  xcb_generic_error_t* error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to change xcb graphics context color\n",
            error_check->error_code);
  }
}
void entis_set_color_rgb(uint32_t r, uint32_t g, uint32_t b) {
  entis_set_color(0x010000 * r + 0x000100 * g + b);
}
void entis_set_color_drgb(double r, double g, double b) {
  entis_set_color(0x010000 * (uint32_t)(255 * r) +
                  0x000100 * (uint32_t)(255 * g) + (uint32_t)(255 * b));
}

void entis_set_background(uint32_t color) {
  uint32_t values[1] = {color};
  xcb_void_cookie_t cookie = xcb_change_window_attributes(
      connection_, window_, XCB_CW_BACK_PIXEL, values);
  xcb_generic_error_t* error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to change xcb window background color\n",
            error_check->error_code);
  }
  free(error_check);
  cookie = xcb_change_gc(connection_, pixmap_bg_gcontext_, XCB_GC_FOREGROUND,
                         values);
  error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to change xcb graphics context color\n",
            error_check->error_code);
  }
  free(error_check);
}
void entis_set_background_rgb(uint32_t r, uint32_t g, uint32_t b) {
  entis_set_background(0x010000 * r + 0x000100 * g + b);
}
void entis_set_background_drgb(double r, double g, double b) {
  entis_set_background(0x010000 * (uint32_t)(255 * r) +
                       0x000100 * (uint32_t)(256 * g) + (uint32_t)(256 * b));
}

void entis_reload_pixmap(uint32_t w, uint32_t h) {
  xcb_free_pixmap(connection_, pixmap_);
  xcb_void_cookie_t cookie = xcb_create_pixmap(connection_, screen_->root_depth,
                                               pixmap_, screen_->root, w, h);
  xcb_generic_error_t* error_check = xcb_request_check(connection_, cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to resize xcb pixmap\n",
            error_check->error_code);
  }
  free(error_check);
  width_ = w;
  height_ = h;
  entis_clear();
}

void entis_copy_pixmap() {
  xcb_void_cookie_t cookie = xcb_copy_area(
      connection_, pixmap_, window_, gcontext_, 0, 0, 0, 0, width_, height_);
  xcb_generic_error_t* error_check =
      xcb_request_check(entis_get_connection(), cookie);
  if (error_check != NULL) {
    fprintf(stderr, "[ERROR %u] Failed to copy pixmap to window\n",
            error_check->error_code);
  }
}

void entis_update() {
  entis_copy_pixmap();
  entis_flush();
}

void entis_clear() {
  xcb_poly_fill_rectangle(connection_, pixmap_, pixmap_bg_gcontext_, 1,
                          (xcb_rectangle_t[]){{0, 0, width_, height_}});
}

void entis_sleep(double sec){
  double seconds;
  double nanosec = modf(sec, &seconds);
  nanosec *= 1e9;
  nanosleep(&(struct timespec){(int)seconds, (int)nanosec}, NULL);
}

entis_event entis_wait_event() {
  entis_update();
  entis_event event = entis_event_wait_event();
  while (true) {
    switch (event.type) {
      case ENTIS_EXPOSE: {
        entis_copy_pixmap();
        entis_flush();
        break;
      }
      case ENTIS_REPARENT_NOTIFY: {
        break;
      }
      case ENTIS_CONFIGURE_NOTIFY: {
        if (event.configure.width != width_ ||
            event.configure.height != height_) {
          entis_reload_pixmap(event.configure.width, event.configure.height);
        }
        break;
      }
      case ENTIS_MAP_NOTIFY: {
        break;
      }
      case ENTIS_NO_EXPOSURE: {
        break;
      }
      default: { return event; }
    }
    event = entis_event_wait_event();
  }
  return event;
}

entis_event entis_poll_event() {
  entis_update();
  entis_event event = entis_event_poll_event();
  while (true) {
    switch (event.type) {
      case ENTIS_EXPOSE: {
        entis_copy_pixmap();
        entis_flush();
        break;
      }
      case ENTIS_REPARENT_NOTIFY: {
        break;
      }
      case ENTIS_CONFIGURE_NOTIFY: {
        if (event.configure.width != width_ ||
            event.configure.height != height_) {
          entis_reload_pixmap(event.configure.width, event.configure.height);
        }
        break;
      }
      case ENTIS_MAP_NOTIFY: {
        break;
      }
      case ENTIS_NO_EXPOSURE: {
        return (entis_event){ENTIS_NO_EVENT};
        break;
      }
      default: { return event; }
    }
    event = entis_event_poll_event();
  }
  return event;
}

entis_event entis_wait_event_type(uint32_t type) {
  entis_event event = entis_wait_event();
  while ((event.type & type) == false) {
    event = entis_wait_event();
  }
  return event;
}

entis_event entis_poll_event_type(uint32_t type) {
  entis_event event = entis_poll_event();
  while ((event.type & type) == false && event.type != ENTIS_NO_EVENT) {
    event = entis_poll_event();
  }
  return event;
}

entis_key_event entis_wait_key() {
  return entis_wait_event_type(ENTIS_KEY_PRESS | ENTIS_KEY_RELEASE).key;
}
entis_key_event entis_poll_key() {
  return entis_poll_event_type(ENTIS_KEY_PRESS | ENTIS_KEY_RELEASE).key;
}

entis_button_event entis_wait_button() {
  return entis_wait_event_type(ENTIS_BUTTON_PRESS | ENTIS_BUTTON_RELEASE)
      .button;
}
entis_button_event entis_poll_button() {
  return entis_poll_event_type(ENTIS_BUTTON_PRESS | ENTIS_BUTTON_RELEASE)
      .button;
}

void entis_clear_events() {
  entis_event event = entis_poll_event();
  while (event.type != ENTIS_NO_EVENT && event.type != ENTIS_NO_EXPOSURE) {
    event = entis_poll_event();
  }
}

void entis_point(uint16_t x, uint16_t y) {
  xcb_poly_point(entis_get_connection(), XCB_COORD_MODE_ORIGIN, pixmap_,
                 pixmap_gcontext_, 1, (xcb_point_t[]){{x, y}});
}
void entis_segment(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  xcb_poly_segment(connection_, pixmap_, pixmap_gcontext_, 1,
                   (xcb_segment_t[]){{x0, y0, x1, y1}});
}
void entis_line(uint16_t* x, uint16_t* y, uint16_t n) {
  xcb_point_t points[n];
  for (int i = 0; i < n; i++) {
    points[i] = (xcb_point_t){x[i], y[i]};
  }
  xcb_poly_line(connection_, XCB_COORD_MODE_ORIGIN, pixmap_, pixmap_gcontext_,
                n, points);
}
void entis_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2) {
  xcb_poly_line(connection_, XCB_COORD_MODE_ORIGIN, pixmap_, pixmap_gcontext_,
                3, (xcb_point_t[]){{x0, y0}, {x1, y1}, {x2, y2}});
}
void entis_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  xcb_poly_rectangle(connection_, pixmap_, pixmap_gcontext_, 1,
                     (xcb_rectangle_t[]){{x, y, width, height}});
}
void entis_arc(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
               uint16_t angle1, uint16_t angle2) {
  xcb_poly_arc(connection_, pixmap_, pixmap_gcontext_, 1,
               (xcb_arc_t[]){{x, y, width, height, angle1, angle2}});
}
void entis_poly(uint16_t* x, uint16_t* y, uint16_t n) {
  xcb_point_t points[n];
  for (int i = 0; i < n; i++) {
    points[i] = (xcb_point_t){x[i], y[i]};
  }
  xcb_poly_line(connection_, XCB_COORD_MODE_ORIGIN, pixmap_, pixmap_gcontext_,
                n, points);
}

void entis_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                         uint16_t x2, uint16_t y2) {
  xcb_fill_poly(connection_, pixmap_, pixmap_gcontext_, XCB_POLY_SHAPE_CONVEX,
                XCB_COORD_MODE_ORIGIN, 3,
                (xcb_point_t[]){{x0, y0}, {x1, y1}, {x2, y2}});
}
void entis_fill_rectangle(uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height) {
  xcb_poly_fill_rectangle(connection_, pixmap_, pixmap_gcontext_, 1,
                          (xcb_rectangle_t[]){{x, y, width, height}});
}
void entis_fill_arc(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                    uint16_t angle1, uint16_t angle2) {
  xcb_poly_fill_arc(connection_, pixmap_, pixmap_gcontext_, 1,
                    (xcb_arc_t[]){{x, y, width, height, angle1, angle2}});
}
void entis_fill_poly(uint16_t* x, uint16_t* y, uint16_t n) {
  xcb_point_t points[n];
  for (int i = 0; i < n; i++) {
    points[i] = (xcb_point_t){x[i], y[i]};
  }
  xcb_fill_poly(connection_, pixmap_, pixmap_gcontext_, XCB_POLY_SHAPE_CONVEX,
                XCB_COORD_MODE_ORIGIN, n, points);
}

void entis_reg_poly(uint16_t x, uint16_t y, uint16_t radius_x,
                    uint16_t radius_y, uint16_t n, double offset) {
  uint16_t points_x[n + 1], points_y[n + 1];
  double delta = 2.0 * M_PI / (double)n;
  int index = 0;
  for (double theta = 0 + offset; theta < (2 * M_PI) + offset;
       theta += delta, index++) {
    points_x[index] = radius_x * cos(theta) + x;
    points_y[index] = radius_y * sin(theta) + y;
  }
  points_x[n] = points_x[0];
  points_y[n] = points_y[0];
  entis_poly(points_x, points_y, n + 1);
}
void entis_fill_reg_poly(uint16_t x, uint16_t y, uint16_t radius_x,
                         uint16_t radius_y, uint16_t n, double offset) {
  uint16_t points_x[n + 1], points_y[n + 1];
  double delta = 2.0 * M_PI / (double)n;
  int index = 0;
  for (double theta = 0 + offset; theta < (2 * M_PI) + offset;
       theta += delta, index++) {
    points_x[index] = radius_x * cos(theta) + x;
    points_y[index] = radius_y * sin(theta) + y;
  }
  points_x[n] = points_x[0];
  points_y[n] = points_y[0];
  entis_fill_poly(points_x, points_y, n + 1);
}

void entis_circle(uint16_t x, uint16_t y, uint16_t radius) {
  entis_reg_poly(x, y, radius, radius, radius, 0);
}
void entis_fill_circle(uint16_t x, uint16_t y, uint16_t radius) {
  entis_fill_reg_poly(x, y, radius, radius, radius, 0);
}

uint16_t entis_get_pixel_width() { return (width_ / pix_width_); }
uint16_t entis_get_pixel_height() { return (height_ / pix_height_); };
void entis_set_pixel_size(uint16_t width, uint16_t height) {
  pix_width_ = width;
  pix_height_ = height;
}
void entis_set_pixel(uint16_t x, uint16_t y) {
  if (pix_width_ * x < width_ && pix_height_ * y < height_) {
    entis_fill_rectangle(x * pix_width_, y * pix_height_, pix_width_,
                         pix_height_);
  }
}

void entis_pixel_set_pixel(uint16_t x, uint16_t y) {
  if (x < width_ && y < height_) {
    x -= (x % pix_width_);
    y -= (y % pix_height_);
    entis_fill_rectangle(x, y, pix_width_, pix_height_);
  }
}
