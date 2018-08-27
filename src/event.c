#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

#include "entis.h"
#include "key_codes.h"

entis_event entis_event_wait_event() {
  entis_event ret;
  xcb_generic_event_t* event = xcb_wait_for_event(entis_get_connection());
  ret = entis_event_parse_event(event);
  free(event);
  return ret;
}
entis_event entis_event_poll_event() {
  entis_event ret;
  xcb_generic_event_t* event = xcb_poll_for_event(entis_get_connection());
  if (event != NULL) {
    ret = entis_event_parse_event(event);
    free(event);
  } else {
    ret.type = ENTIS_NO_EVENT;
  }
  return ret;
}

enum EventType entis_event_parse_type(xcb_generic_event_t* event) {
  switch (event->response_type & ~0x80) {
    case XCB_KEY_PRESS:
      return ENTIS_KEY_PRESS;
    case XCB_KEY_RELEASE:
      return ENTIS_KEY_RELEASE;
    case XCB_BUTTON_PRESS:
      return ENTIS_BUTTON_PRESS;
    case XCB_BUTTON_RELEASE:
      return ENTIS_BUTTON_RELEASE;
    case XCB_MOTION_NOTIFY:
      return ENTIS_MOTION_NOTIFY;
    case XCB_ENTER_NOTIFY:
      return ENTIS_ENTER_NOTIFY;
    case XCB_LEAVE_NOTIFY:
      return ENTIS_LEAVE_NOTIFY;
    case XCB_FOCUS_IN:
      return ENTIS_FOCUS_IN;
    case XCB_FOCUS_OUT:
      return ENTIS_FOCUS_OUT;
    case XCB_KEYMAP_NOTIFY:
      return ENTIS_KEYMAP_NOTIFY;
    case XCB_EXPOSE:
      return ENTIS_EXPOSE;
    case XCB_GRAPHICS_EXPOSURE:
      return ENTIS_GRAPHICS_EXPOSURE;
    case XCB_NO_EXPOSURE:
      return ENTIS_NO_EXPOSURE;
    case XCB_VISIBILITY_NOTIFY:
      return ENTIS_VISIBILITY_NOTIFY;
    case XCB_CREATE_NOTIFY:
      return ENTIS_CREATE_NOTIFY;
    case XCB_DESTROY_NOTIFY:
      return ENTIS_DESTROY_NOTIFY;
    case XCB_UNMAP_NOTIFY:
      return ENTIS_UNMAP_NOTIFY;
    case XCB_MAP_NOTIFY:
      return ENTIS_MAP_NOTIFY;
    case XCB_MAP_REQUEST:
      return ENTIS_MAP_REQUEST;
    case XCB_REPARENT_NOTIFY:
      return ENTIS_REPARENT_NOTIFY;
    case XCB_CONFIGURE_NOTIFY:
      return ENTIS_CONFIGURE_NOTIFY;
    case XCB_CONFIGURE_REQUEST:
      return ENTIS_CONFIGURE_REQUEST;
    case XCB_GRAVITY_NOTIFY:
      return ENTIS_GRAVITY_NOTIFY;
    case XCB_RESIZE_REQUEST:
      return ENTIS_RESIZE_REQUEST;
    case XCB_CIRCULATE_NOTIFY:
      return ENTIS_CIRCULATE_NOTIFY;
    case XCB_CIRCULATE_REQUEST:
      return ENTIS_CIRCULATE_REQUEST;
    case XCB_PROPERTY_NOTIFY:
      return ENTIS_PROPERTY_NOTIFY;
    case XCB_SELECTION_CLEAR:
      return ENTIS_SELECTION_CLEAR;
    case XCB_SELECTION_REQUEST:
      return ENTIS_SELECTION_REQUEST;
    case XCB_SELECTION_NOTIFY:
      return ENTIS_SELECTION_NOTIFY;
    case XCB_CLIENT_MESSAGE:
      return ENTIS_CLIENT_MESSAGE;
    case XCB_MAPPING_NOTIFY:
      return ENTIS_MAPPING_NOTIFY;
    case XCB_GE_GENERIC:
      return ENTIS_GE_GENERIC;
    default:
      return ENTIS_NO_EVENT;
  }
  return ENTIS_NO_EVENT;
}

entis_key_event entis_event_parse_key(xcb_generic_event_t* event,
                                      enum EventType type) {
  if (type == ENTIS_KEY_PRESS) {
    xcb_key_press_event_t* ev = (xcb_key_press_event_t*)event;
    return (entis_key_event){
        type,
        ev->time,
        ev->event_x,
        ev->event_y,
        ev->root_x,
        ev->root_y,
        ev->state,
        entis_parse_keycode(ev->detail),
        entis_keycode_to_keysym(entis_parse_keycode(ev->detail), ev->state)};
    entis_set_key_state(entis_parse_keycode(ev->detail), true);
  } else if (type == ENTIS_KEY_RELEASE) {
    xcb_key_release_event_t* ev = (xcb_key_release_event_t*)event;
    return (entis_key_event){
        type,
        ev->time,
        ev->event_x,
        ev->event_y,
        ev->root_x,
        ev->root_y,
        ev->state,
        entis_parse_keycode(ev->detail),
        entis_keycode_to_keysym(entis_parse_keycode(ev->detail), ev->state)};
    entis_set_key_state(entis_parse_keycode(ev->detail), false);
  } else {
    return (entis_key_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_button_event entis_event_parse_button(xcb_generic_event_t* event,
                                            enum EventType type) {
  if (type == ENTIS_BUTTON_PRESS) {
    xcb_button_press_event_t* ev = (xcb_button_press_event_t*)event;
    return (entis_button_event){type,        ev->time,   ev->event_x,
                                ev->event_y, ev->root_x, ev->root_y,
                                ev->state,   ev->detail};
  } else if (type == ENTIS_BUTTON_RELEASE) {
    xcb_button_release_event_t* ev = (xcb_button_release_event_t*)event;
    return (entis_button_event){type,        ev->time,   ev->event_x,
                                ev->event_y, ev->root_x, ev->root_y,
                                ev->state,   ev->detail};
  } else {
    return (entis_button_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_motion_event entis_event_parse_motion(xcb_generic_event_t* event,
                                            enum EventType type) {
  if (type == ENTIS_MOTION_NOTIFY) {
    xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*)event;
    return (entis_motion_event){type,        ev->time,   ev->event_x,
                                ev->event_y, ev->root_x, ev->root_y,
                                ev->state,   ev->detail};
  } else {
    return (entis_motion_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_crossing_event entis_event_parse_crossing(xcb_generic_event_t* event,
                                                enum EventType type) {
  if (type == ENTIS_ENTER_NOTIFY) {
    xcb_enter_notify_event_t* ev = (xcb_enter_notify_event_t*)event;
    return (entis_crossing_event){type,        ev->time,   ev->event_x,
                                  ev->event_y, ev->root_x, ev->root_y,
                                  ev->state,   ev->event};
  } else if (type == ENTIS_LEAVE_NOTIFY) {
    xcb_leave_notify_event_t* ev = (xcb_leave_notify_event_t*)event;
    return (entis_crossing_event){type,        ev->time,   ev->event_x,
                                  ev->event_y, ev->root_x, ev->root_y,
                                  ev->state,   ev->event};
  } else {
    return (entis_crossing_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_focus_event entis_event_parse_focus(xcb_generic_event_t* event,
                                          enum EventType type) {
  if (type == ENTIS_FOCUS_IN) {
    xcb_focus_in_event_t* ev = (xcb_focus_in_event_t*)event;
    return (entis_focus_event){type, ev->mode, ev->detail};
  } else if (type == ENTIS_FOCUS_OUT) {
    xcb_focus_out_event_t* ev = (xcb_focus_out_event_t*)event;
    return (entis_focus_event){type, ev->mode, ev->detail};
  } else {
    return (entis_focus_event){ENTIS_NO_EVENT, 0, 0};
  }
}
entis_expose_event entis_event_parse_expose(xcb_generic_event_t* event,
                                            enum EventType type) {
  if (type == ENTIS_EXPOSE) {
    xcb_expose_event_t* ev = (xcb_expose_event_t*)event;
    return (entis_expose_event){type,      ev->x,      ev->y,
                                ev->width, ev->height, ev->count};
  } else {
    return (entis_expose_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0};
  }
}
entis_graphics_expose_event entis_event_parse_graphics_expose(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_GRAPHICS_EXPOSURE) {
    xcb_graphics_exposure_event_t* ev = (xcb_graphics_exposure_event_t*)event;
    return (entis_graphics_expose_event){type,
                                         ev->x,
                                         ev->y,
                                         ev->width,
                                         ev->height,
                                         ev->major_opcode,
                                         ev->minor_opcode,
                                         ev->count};
  } else {
    return (entis_graphics_expose_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_no_expose_event entis_event_parse_no_expose(xcb_generic_event_t* event,
                                                  enum EventType type) {
  if (type == ENTIS_NO_EXPOSURE) {
    xcb_no_exposure_event_t* ev = (xcb_no_exposure_event_t*)event;
    return (entis_no_expose_event){type, ev->major_opcode, ev->minor_opcode};
  } else {
    return (entis_no_expose_event){ENTIS_NO_EVENT, 0, 0};
  }
}
entis_visibility_event entis_event_parse_visibility(xcb_generic_event_t* event,
                                                    enum EventType type) {
  if (type == ENTIS_VISIBILITY_NOTIFY) {
    xcb_visibility_notify_event_t* ev = (xcb_visibility_notify_event_t*)event;
    return (entis_visibility_event){type, ev->state};
  } else {
    return (entis_visibility_event){ENTIS_NO_EVENT, 0};
  }
}
entis_create_event entis_event_parse_create(xcb_generic_event_t* event,
                                            enum EventType type) {
  if (type == ENTIS_CREATE_NOTIFY) {
    xcb_create_notify_event_t* ev = (xcb_create_notify_event_t*)event;
    return (entis_create_event){type,      ev->x,      ev->y,
                                ev->width, ev->height, ev->border_width};
  } else {
    return (entis_create_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0};
  }
}
entis_destroy_event entis_event_parse_destroy(xcb_generic_event_t* event,
                                              enum EventType type) {
  if (type == ENTIS_DESTROY_NOTIFY) {
    xcb_destroy_notify_event_t* ev = (xcb_destroy_notify_event_t*)event;
    return (entis_destroy_event){type, ev->event};
  } else {
    return (entis_destroy_event){ENTIS_NO_EVENT, 0};
  }
}
entis_unmap_event entis_event_parse_unmap(xcb_generic_event_t* event,
                                          enum EventType type) {
  if (type == ENTIS_UNMAP_NOTIFY) {
    xcb_unmap_notify_event_t* ev = (xcb_unmap_notify_event_t*)event;
    return (entis_unmap_event){type, ev->event};
  } else {
    return (entis_unmap_event){ENTIS_NO_EVENT, 0};
  }
}
entis_map_event entis_event_parse_map(xcb_generic_event_t* event,
                                      enum EventType type) {
  if (type == ENTIS_MAP_NOTIFY) {
    xcb_map_notify_event_t* ev = (xcb_map_notify_event_t*)event;
    return (entis_map_event){type, ev->event};
  } else {
    return (entis_map_event){ENTIS_NO_EVENT, 0};
  }
}
entis_map_request_event entis_event_parse_map_request(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_MAP_REQUEST) {
    xcb_map_request_event_t* ev = (xcb_map_request_event_t*)event;
    return (entis_map_request_event){type, ev->parent};
  } else {
    return (entis_map_request_event){ENTIS_NO_EVENT, 0};
  }
}
entis_reparent_event entis_event_parse_reparent(xcb_generic_event_t* event,
                                                enum EventType type) {
  if (type == ENTIS_REPARENT_NOTIFY) {
    xcb_reparent_notify_event_t* ev = (xcb_reparent_notify_event_t*)event;
    return (entis_reparent_event){type, ev->event, ev->parent, ev->x, ev->y};
  } else {
    return (entis_reparent_event){ENTIS_NO_EVENT, 0, 0, 0, 0};
  }
}
entis_configure_event entis_event_parse_configure(xcb_generic_event_t* event,
                                                  enum EventType type) {
  if (type == ENTIS_CONFIGURE_NOTIFY) {
    xcb_configure_notify_event_t* ev = (xcb_configure_notify_event_t*)event;
    return (entis_configure_event){
        type, ev->event, ev->x, ev->y, ev->width, ev->height, ev->border_width};
  } else {
    return (entis_configure_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0};
  }
}
entis_gravity_event entis_event_parse_gravity(xcb_generic_event_t* event,
                                              enum EventType type) {
  if (type == ENTIS_GRAVITY_NOTIFY) {
    xcb_gravity_notify_event_t* ev = (xcb_gravity_notify_event_t*)event;
    return (entis_gravity_event){type, ev->event, ev->x, ev->y};
  } else {
    return (entis_gravity_event){ENTIS_NO_EVENT, 0, 0, 0};
  }
}
entis_resize_event entis_event_parse_resize(xcb_generic_event_t* event,
                                            enum EventType type) {
  if (type == ENTIS_RESIZE_REQUEST) {
    xcb_resize_request_event_t* ev = (xcb_resize_request_event_t*)event;
    return (entis_resize_event){type, ev->window, ev->width, ev->height};
  } else {
    return (entis_resize_event){ENTIS_NO_EVENT, 0, 0, 0};
  }
}
entis_configure_request_event entis_event_parse_configure_request(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_CONFIGURE_REQUEST) {
    xcb_configure_request_event_t* ev = (xcb_configure_request_event_t*)event;
    return (entis_configure_request_event){
        type,      ev->window, ev->sibling,      ev->x,         ev->y,
        ev->width, ev->height, ev->border_width, ev->value_mask};
  } else {
    return (entis_configure_request_event){
        ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0, 0, 0};
  }
}
entis_circulate_event entis_event_parse_circulate(xcb_generic_event_t* event,
                                                  enum EventType type) {
  if (type == ENTIS_CIRCULATE_NOTIFY) {
    xcb_circulate_notify_event_t* ev = (xcb_circulate_notify_event_t*)event;
    return (entis_circulate_event){type, ev->place};
  } else {
    return (entis_circulate_event){ENTIS_NO_EVENT, 0};
  }
}
entis_circulate_request_event entis_event_parse_circulate_request(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_CIRCULATE_REQUEST) {
    xcb_circulate_request_event_t* ev = (xcb_circulate_request_event_t*)event;
    return (entis_circulate_request_event){type, ev->place};
  } else {
    return (entis_circulate_request_event){ENTIS_NO_EVENT, 0};
  }
}
entis_property_event entis_event_parse_property(xcb_generic_event_t* event,
                                                enum EventType type) {
  if (type == ENTIS_PROPERTY_NOTIFY) {
    xcb_property_notify_event_t* ev = (xcb_property_notify_event_t*)event;
    return (entis_property_event){type, ev->time, ev->atom, ev->state};
  } else {
    return (entis_property_event){ENTIS_NO_EVENT, 0, 0, 0};
  }
}
entis_selection_event entis_event_parse_selection(xcb_generic_event_t* event,
                                                  enum EventType type) {
  if (type == ENTIS_SELECTION_NOTIFY) {
    xcb_selection_notify_event_t* ev = (xcb_selection_notify_event_t*)event;
    return (entis_selection_event){type,       ev->requestor, ev->selection,
                                   ev->target, ev->property,  ev->time};
  } else {
    return (entis_selection_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0};
  }
}
entis_selection_clear_event entis_event_parse_selection_clear(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_SELECTION_CLEAR) {
    xcb_selection_clear_event_t* ev = (xcb_selection_clear_event_t*)event;
    return (entis_selection_clear_event){type, ev->selection, ev->time};
  } else {
    return (entis_selection_clear_event){ENTIS_NO_EVENT, 0, 0};
  }
}
entis_selection_request_event entis_event_parse_selection_request(
    xcb_generic_event_t* event, enum EventType type) {
  if (type == ENTIS_SELECTION_REQUEST) {
    xcb_selection_request_event_t* ev = (xcb_selection_request_event_t*)event;
    return (entis_selection_request_event){
        type,       ev->owner,    ev->requestor, ev->selection,
        ev->target, ev->property, ev->time};
  } else {
    return (entis_selection_request_event){ENTIS_NO_EVENT, 0, 0, 0, 0, 0, 0};
  }
}
entis_mapping_event entis_event_parse_mapping(xcb_generic_event_t* event,
                                              enum EventType type) {
  if (type == ENTIS_MAPPING_NOTIFY) {
    xcb_mapping_notify_event_t* ev = (xcb_mapping_notify_event_t*)event;
    return (entis_mapping_event){type, ev->request, ev->first_keycode,
                                 ev->count};
  } else {
    return (entis_mapping_event){ENTIS_NO_EVENT, 0, 0, 0};
  }
}

entis_event entis_event_parse_event(xcb_generic_event_t* event) {
  entis_event ret;
  ret.type = entis_event_parse_type(event);
  switch (entis_event_parse_type(event)) {
    case ENTIS_KEY_PRESS:
      ret.key = entis_event_parse_key(event, ENTIS_KEY_PRESS);
      break;
    case ENTIS_KEY_RELEASE:
      ret.key = entis_event_parse_key(event, ENTIS_KEY_RELEASE);
      break;
    case ENTIS_BUTTON_PRESS:
      ret.button = entis_event_parse_button(event, ENTIS_BUTTON_PRESS);
      break;
    case ENTIS_BUTTON_RELEASE:
      ret.button = entis_event_parse_button(event, ENTIS_BUTTON_RELEASE);
      break;
    case ENTIS_MOTION_NOTIFY:
      ret.motion = entis_event_parse_motion(event, ENTIS_MOTION_NOTIFY);
      break;
    case ENTIS_ENTER_NOTIFY:
      ret.crossing = entis_event_parse_crossing(event, ENTIS_ENTER_NOTIFY);
      break;
    case ENTIS_LEAVE_NOTIFY:
      ret.crossing = entis_event_parse_crossing(event, ENTIS_LEAVE_NOTIFY);
      break;
    case ENTIS_FOCUS_IN:
      ret.focus = entis_event_parse_focus(event, ENTIS_FOCUS_IN);
      break;
    case ENTIS_FOCUS_OUT:
      ret.focus = entis_event_parse_focus(event, ENTIS_FOCUS_OUT);
      break;
    case ENTIS_KEYMAP_NOTIFY:
      /* ret.keymap = entis_event_parse_keymap(event, ENTIS_KEYMAP_NOTIFY); */
      break;
    case ENTIS_EXPOSE:
      ret.expose = entis_event_parse_expose(event, ENTIS_EXPOSE);
      break;
    case ENTIS_GRAPHICS_EXPOSURE:
      ret.graphics_expose =
          entis_event_parse_graphics_expose(event, ENTIS_GRAPHICS_EXPOSURE);
      break;
    case ENTIS_NO_EXPOSURE:
      ret.no_expose = entis_event_parse_no_expose(event, ENTIS_NO_EXPOSURE);
      break;
    case ENTIS_VISIBILITY_NOTIFY:
      ret.visibility =
          entis_event_parse_visibility(event, ENTIS_VISIBILITY_NOTIFY);
      break;
    case ENTIS_CREATE_NOTIFY:
      ret.create = entis_event_parse_create(event, ENTIS_CREATE_NOTIFY);
      break;
    case ENTIS_DESTROY_NOTIFY:
      ret.destroy = entis_event_parse_destroy(event, ENTIS_DESTROY_NOTIFY);
      break;
    case ENTIS_UNMAP_NOTIFY:
      ret.unmap = entis_event_parse_unmap(event, ENTIS_UNMAP_NOTIFY);
      break;
    case ENTIS_MAP_NOTIFY:
      ret.map = entis_event_parse_map(event, ENTIS_MAP_NOTIFY);
      break;
    case ENTIS_MAP_REQUEST:
      ret.map_request = entis_event_parse_map_request(event, ENTIS_MAP_REQUEST);
      break;
    case ENTIS_REPARENT_NOTIFY:
      ret.reparent = entis_event_parse_reparent(event, ENTIS_REPARENT_NOTIFY);
      break;
    case ENTIS_CONFIGURE_NOTIFY:
      ret.configure =
          entis_event_parse_configure(event, ENTIS_CONFIGURE_NOTIFY);
      break;
    case ENTIS_CONFIGURE_REQUEST:
      ret.configure_request =
          entis_event_parse_configure_request(event, ENTIS_CONFIGURE_REQUEST);
      break;
    case ENTIS_GRAVITY_NOTIFY:
      ret.gravity = entis_event_parse_gravity(event, ENTIS_GRAVITY_NOTIFY);
      break;
    case ENTIS_RESIZE_REQUEST:
      ret.resize = entis_event_parse_resize(event, ENTIS_RESIZE_REQUEST);
      break;
    case ENTIS_CIRCULATE_NOTIFY:
      ret.circulate =
          entis_event_parse_circulate(event, ENTIS_CIRCULATE_NOTIFY);
      break;
    case ENTIS_CIRCULATE_REQUEST:
      ret.circulate_request =
          entis_event_parse_circulate_request(event, ENTIS_CIRCULATE_REQUEST);
      break;
    case ENTIS_PROPERTY_NOTIFY:
      ret.property = entis_event_parse_property(event, ENTIS_PROPERTY_NOTIFY);
      break;
    case ENTIS_SELECTION_CLEAR:
      ret.selection_clear =
          entis_event_parse_selection_clear(event, ENTIS_SELECTION_CLEAR);
      break;
    case ENTIS_SELECTION_REQUEST:
      ret.selection_request =
          entis_event_parse_selection_request(event, ENTIS_SELECTION_REQUEST);
      break;
    case ENTIS_SELECTION_NOTIFY:
      ret.selection =
          entis_event_parse_selection(event, ENTIS_SELECTION_NOTIFY);
      break;
    case ENTIS_MAPPING_NOTIFY:
      ret.mapping = entis_event_parse_mapping(event, ENTIS_MAPPING_NOTIFY);
      break;
    default:
      ret.type = ENTIS_NO_EVENT;
      break;
  }
  return ret;
}
