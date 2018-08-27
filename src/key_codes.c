#include "key_codes.h"

#include <inttypes.h>
#include <stdbool.h>

bool key_state[300];

uint16_t entis_parse_keycode(uint16_t keycode) {
  switch (keycode) {
    case 65:
      return KEY_SPACE;
    case 48:
      return KEY_APOSTROPHE;
    case 59:
      return KEY_COMMA;
    case 20:
      return KEY_MINUS;
    case 60:
      return KEY_PERIOD;
    case 61:
      return KEY_SLASH;
    case 19:
      return KEY_0;
    case 10:
      return KEY_1;
    case 11:
      return KEY_2;
    case 12:
      return KEY_3;
    case 13:
      return KEY_4;
    case 14:
      return KEY_5;
    case 15:
      return KEY_6;
    case 16:
      return KEY_7;
    case 17:
      return KEY_8;
    case 18:
      return KEY_9;
    case 47:
      return KEY_SEMICOLON;
    case 21:
      return KEY_EQUAL;
    case 38:
      return KEY_A;
    case 56:
      return KEY_B;
    case 54:
      return KEY_C;
    case 40:
      return KEY_D;
    case 26:
      return KEY_E;
    case 41:
      return KEY_F;
    case 42:
      return KEY_G;
    case 43:
      return KEY_H;
    case 31:
      return KEY_I;
    case 44:
      return KEY_J;
    case 45:
      return KEY_K;
    case 46:
      return KEY_L;
    case 58:
      return KEY_M;
    case 57:
      return KEY_N;
    case 32:
      return KEY_O;
    case 33:
      return KEY_P;
    case 24:
      return KEY_Q;
    case 27:
      return KEY_R;
    case 39:
      return KEY_S;
    case 28:
      return KEY_T;
    case 30:
      return KEY_U;
    case 55:
      return KEY_V;
    case 25:
      return KEY_W;
    case 53:
      return KEY_X;
    case 29:
      return KEY_Y;
    case 52:
      return KEY_Z;
    case 34:
      return KEY_LEFT_BRACKET;
    case 51:
      return KEY_BACKSLASH;
    case 35:
      return KEY_RIGHT_BRACKET;
    case 49:
      return KEY_GRAVE_ACCENT;
    case 9:
      return KEY_ESCAPE;
    case 36:
      return KEY_ENTER;
    case 23:
      return KEY_TAB;
    case 22:
      return KEY_BACKSPACE;
    case 118:
      return KEY_INSERT;
    case 119:
      return KEY_DELETE;
    case 114:
      return KEY_RIGHT;
    case 113:
      return KEY_LEFT;
    case 116:
      return KEY_DOWN;
    case 111:
      return KEY_UP;
    case 117:
      return KEY_PAGE_DOWN;
    case 112:
      return KEY_PAGE_UP;
    case 110:
      return KEY_HOME;
    case 115:
      return KEY_END;
    case 66:
      return KEY_CAPS_LOCK;
    case 255:
      return KEY_PRINT_SCREEN;
    default:
      return 0;
  }
  return keycode;
}

uint16_t entis_keycode_to_keysym(uint16_t keycode, uint16_t state) {
  bool flags[6] = {false, false, false, false, false, false};
  flags[0] = (state & SHIFT);
  flags[1] = (state & CAPS_LOCK);
  flags[2] = (state & CTRL);
  flags[3] = (state & ALT);
  flags[5] = (state & WIN);
  bool should_cap = (flags[0] ^ flags[1]);
  uint16_t sym = 0;
  if (keycode >= 65 && keycode <= 90) {
    // LETTERS
    if (should_cap == true) {
      sym = keycode;
    } else {
      sym = keycode + 32;
    }
  } else if (keycode >= 48 && keycode <= 57) {
    // NUMBERS
    if (should_cap == true) {
      if (keycode >= 49 && keycode <= 53) {
        sym = keycode - 16;
      } else if (keycode == 48) {
        sym = 41;
      } else if (keycode == 54) {
        sym = 94;
      } else if (keycode == 55) {
        sym = 38;
      } else if (keycode == 56) {
        sym = 42;
      } else if (keycode == 57) {
        sym = 40;
      }
    } else {
      sym = keycode;
    }
  } else if (keycode >= 39 && keycode <= 47) {
    // PREV NUMBER
    if (should_cap == true) {
      if (keycode == 39) {
        sym = 34;
      } else if (keycode == 44) {
        sym = 60;
      } else if (keycode == 45) {
        sym = 95;
      } else if (keycode == 46) {
        sym = 62;
      } else if (keycode == 47) {
        sym = 63;
      }
    } else {
      sym = keycode;
    }
  } else if (keycode >= 59 && keycode <= 61) {
    // NUM-ALPHA
    if (should_cap == true) {
      if (keycode == 59) {
        sym = 58;
      } else if (keycode == 61) {
        sym = 43;
      }
    } else {
      sym = keycode;
    }
  } else if (keycode >= 91 && keycode <= 93) {
    // BRACKET
    if (should_cap == true) {
      sym = keycode + 32;
    } else {
      sym = keycode;
    }
  } else if (keycode == 96) {
    // ` or ~
    if (should_cap == true) {
      sym = 126;
    } else {
      sym = keycode;
    }
  } else {
    sym = keycode;
  }
  return sym;
}

bool entis_key_pressed(uint16_t keycode){
  return key_state[keycode];
}
void entis_set_key_state(uint16_t keycode, bool state){
  key_state[keycode] = state;
}
