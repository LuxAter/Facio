#ifndef ENTIS_EVENT_H_
#define ENTIS_EVENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <xcb/xcb.h>

enum EventType {
  ENTIS_NO_EVENT = (1lu << 0),
  ENTIS_KEY_PRESS = (1lu << 1),
  ENTIS_KEY_RELEASE = (1lu << 2),
  ENTIS_BUTTON_PRESS = (1lu << 3),
  ENTIS_BUTTON_RELEASE = (1lu << 4),
  ENTIS_MOTION_NOTIFY = (1lu << 5),
  ENTIS_ENTER_NOTIFY = (1lu << 6),
  ENTIS_LEAVE_NOTIFY = (1lu << 7),
  ENTIS_FOCUS_IN = (1lu << 8),
  ENTIS_FOCUS_OUT = (1lu << 9),
  ENTIS_KEYMAP_NOTIFY = (1lu << 10),
  ENTIS_EXPOSE = (1lu << 11),
  ENTIS_GRAPHICS_EXPOSURE = (1lu << 12),
  ENTIS_NO_EXPOSURE = (1lu << 13),
  ENTIS_VISIBILITY_NOTIFY = (1lu << 14),
  ENTIS_CREATE_NOTIFY = (1lu << 15),
  ENTIS_DESTROY_NOTIFY = (1lu << 16),
  ENTIS_UNMAP_NOTIFY = (1lu << 17),
  ENTIS_MAP_NOTIFY = (1lu << 18),
  ENTIS_MAP_REQUEST = (1lu << 19),
  ENTIS_REPARENT_NOTIFY = (1lu << 20),
  ENTIS_CONFIGURE_NOTIFY = (1lu << 21),
  ENTIS_CONFIGURE_REQUEST = (1lu << 22),
  ENTIS_GRAVITY_NOTIFY = (1lu << 23),
  ENTIS_RESIZE_REQUEST = (1lu << 24),
  ENTIS_CIRCULATE_NOTIFY = (1lu << 25),
  ENTIS_CIRCULATE_REQUEST = (1lu << 26),
  ENTIS_PROPERTY_NOTIFY = (1lu << 27),
  ENTIS_SELECTION_CLEAR = (1lu << 28),
  ENTIS_SELECTION_REQUEST = (1lu << 29),
  ENTIS_SELECTION_NOTIFY = (1lu << 30),
  ENTIS_CLIENT_MESSAGE = (1lu << 31),
  ENTIS_MAPPING_NOTIFY = (1lu << 32),
  ENTIS_GE_GENERIC = (1lu << 33)
};

typedef struct entis_key_event {
  enum EventType type;
  uint32_t time;
  int16_t x, y;
  int16_t root_x, root_y;
  uint16_t state;
  uint16_t keycode;
  uint16_t keysym;
} entis_key_event;
typedef struct entis_button_event {
  enum EventType type;
  uint32_t time;
  int16_t x, y;
  int16_t root_x, root_y;
  uint16_t state;
  uint8_t button;
} entis_button_event;
typedef struct entis_motion_event {
  enum EventType type;
  uint32_t time;
  int16_t x, y;
  int16_t root_x, root_y;
  uint16_t state;
  uint8_t detail;
} entis_motion_event;
typedef struct entis_crossing_event {
  enum EventType type;
  uint32_t time;
  int16_t x, y;
  int16_t root_x, root_y;
  uint16_t state;
  uint8_t button;
} entis_crossing_event;
typedef struct entis_focus_event {
  enum EventType type;
  uint8_t mode;
  uint8_t detail;
} entis_focus_event;
typedef struct entis_expose_event {
  enum EventType type;
  uint16_t x, y;
  uint16_t width, height;
  uint16_t count;
} entis_expose_event;
typedef struct entis_graphics_expose_event {
  enum EventType type;
  uint16_t x, y;
  uint16_t width, height;
  uint16_t major, minor;
  uint16_t count;
} entis_graphics_expose_event;
typedef struct entis_no_expose_event {
  enum EventType type;
  uint16_t major, minor;
} entis_no_expose_event;
typedef struct entis_visibility_event {
  enum EventType type;
  uint8_t state;
} entis_visibility_event;
typedef struct entis_create_event {
  enum EventType type;
  uint16_t x, y;
  uint16_t width, height;
  uint16_t border_width;
} entis_create_event;
typedef struct entis_destroy_event {
  enum EventType type;
  xcb_window_t window;
} entis_destroy_event;
typedef struct entis_unmap_event {
  enum EventType type;
  xcb_window_t window;
} entis_unmap_event;
typedef struct entis_map_event {
  enum EventType type;
  xcb_window_t window;
} entis_map_event;
typedef struct entis_map_request_event {
  enum EventType type;
  xcb_window_t parent;
} entis_map_request_event;
typedef struct entis_reparent_event {
  enum EventType type;
  xcb_window_t window, parent;
  uint16_t x, y;
} entis_reparent_event;
typedef struct entis_configure_event {
  enum EventType type;
  xcb_window_t window;
  uint16_t x, y;
  uint16_t width, height;
  uint16_t border_width;
} entis_configure_event;
typedef struct entis_gravity_event {
  enum EventType type;
  xcb_window_t window;
  int16_t x, y;
} entis_gravity_event;
typedef struct entis_resize_event {
  enum EventType type;
  xcb_window_t window;
  int16_t width, height;
} entis_resize_event;
typedef struct entis_configure_request_event {
  enum EventType type;
  xcb_window_t window, sibling;
  uint16_t x, y;
  uint16_t width, height;
  uint16_t border_width;
  uint16_t value_mask;
} entis_configure_request_event;
typedef struct entis_circulate_event {
  enum EventType type;
  uint8_t place;
} entis_circulate_event;
typedef struct entis_circulate_request_event {
  enum EventType type;
  uint8_t place;
} entis_circulate_request_event;
typedef struct entis_property_event {
  enum EventType type;
  uint32_t time;
  uint32_t atom;
  uint8_t state;
} entis_property_event;
typedef struct entis_selection_clear_event {
  enum EventType type;
  uint32_t selection;
  uint32_t time;
} entis_selection_clear_event;
typedef struct entis_selection_request_event {
  enum EventType type;
  xcb_window_t owner, requestor;
  uint32_t selection, target, property;
  uint32_t time;
} entis_selection_request_event;
typedef struct entis_selection_event {
  enum EventType type;
  xcb_window_t requestor;
  uint32_t selection, target, property;
  uint32_t time;
} entis_selection_event;
typedef struct entis_mapping_event {
  enum EventType type;
  uint8_t request, first_keycode, count;
} entis_mapping_event;

typedef struct entis_event {
  enum EventType type;
  union {
    entis_key_event key;
    entis_button_event button;
    entis_motion_event motion;
    entis_crossing_event crossing;
    entis_focus_event focus;
    entis_expose_event expose;
    entis_graphics_expose_event graphics_expose;
    entis_no_expose_event no_expose;
    entis_visibility_event visibility;
    entis_create_event create;
    entis_destroy_event destroy;
    entis_unmap_event unmap;
    entis_map_event map;
    entis_map_request_event map_request;
    entis_reparent_event reparent;
    entis_configure_event configure;
    entis_gravity_event gravity;
    entis_resize_event resize;
    entis_configure_request_event configure_request;
    entis_circulate_event circulate;
    entis_circulate_request_event circulate_request;
    entis_property_event property;
    entis_selection_event selection;
    entis_selection_clear_event selection_clear;
    entis_selection_request_event selection_request;
    entis_mapping_event mapping;
  };
} entis_event;

entis_event entis_event_wait_event();
entis_event entis_event_poll_event();
enum EventType entis_event_parse_type(xcb_generic_event_t* event);
entis_key_event entis_event_parse_key(xcb_generic_event_t* event,
                                      enum EventType type);
entis_button_event entis_event_parse_button(xcb_generic_event_t* event,
                                            enum EventType type);
entis_motion_event entis_event_parse_motion(xcb_generic_event_t* event,
                                            enum EventType type);
entis_crossing_event entis_event_parse_crossing(xcb_generic_event_t* event,
                                                enum EventType type);
entis_focus_event entis_event_parse_focus(xcb_generic_event_t* event,
                                          enum EventType type);
entis_expose_event entis_event_parse_expose(xcb_generic_event_t* event,
                                            enum EventType type);
entis_graphics_expose_event entis_event_parse_graphics_expose(
    xcb_generic_event_t* event, enum EventType type);
entis_no_expose_event entis_event_parse_no_expose(xcb_generic_event_t* event,
                                                  enum EventType type);
entis_visibility_event entis_event_parse_visibility(xcb_generic_event_t* event,
                                                    enum EventType type);
entis_create_event entis_event_parse_create(xcb_generic_event_t* event,
                                            enum EventType type);
entis_destroy_event entis_event_parse_destroy(xcb_generic_event_t* event,
                                              enum EventType type);
entis_unmap_event entis_event_parse_unmap(xcb_generic_event_t* event,
                                          enum EventType type);
entis_map_event entis_event_parse_map(xcb_generic_event_t* event,
                                      enum EventType type);
entis_map_request_event entis_event_parse_map_request(
    xcb_generic_event_t* event, enum EventType type);
entis_reparent_event entis_event_parse_reparent(xcb_generic_event_t* event,
                                                enum EventType type);
entis_configure_event entis_event_parse_configure(xcb_generic_event_t* event,
                                                  enum EventType type);
entis_gravity_event entis_event_parse_gravity(xcb_generic_event_t* event,
                                              enum EventType type);
entis_resize_event entis_event_parse_resize(xcb_generic_event_t* event,
                                            enum EventType type);
entis_configure_request_event entis_event_parse_configure_request(
    xcb_generic_event_t* event, enum EventType type);
entis_circulate_event entis_event_parse_circulate(xcb_generic_event_t* event,
                                                  enum EventType type);
entis_circulate_request_event entis_event_parse_circulate_request(
    xcb_generic_event_t* event, enum EventType type);
entis_property_event entis_event_parse_property(xcb_generic_event_t* event,
                                                enum EventType type);
entis_selection_event entis_event_parse_selection(xcb_generic_event_t* event,
                                                  enum EventType type);
entis_selection_clear_event entis_event_parse_selection_clear(
    xcb_generic_event_t* event, enum EventType type);
entis_selection_request_event entis_event_parse_selection_request(
    xcb_generic_event_t* event, enum EventType type);
entis_mapping_event entis_event_parse_mapping(xcb_generic_event_t* event,
                                              enum EventType type);
entis_event entis_event_parse_event(xcb_generic_event_t* event);

#ifdef __cplusplus
}
#endif

#endif  // ENTIS_EVENT_H_
