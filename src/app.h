

#ifndef APP_H
#define APP_H

enum {
    INPUT_STATE_DOWN = (1 << 0),
    INPUT_STATE_RELEASE = (1 << 1),
    INPUT_STATE_PRESSED = (1 << 2),
    INPUT_STATE_DOUBLE = (1 << 3),
    INPUT_STATE_CTRL = (1 << 4),
    INPUT_STATE_SHIFT = (1 << 5),
    INPUT_STATE_ALT = (1 << 6),
    INPUT_STATE_META = (1 << 7)
};

typedef enum key_code_t {
#define VK_KEY(x) KEY_##x,
#include "keys.inc"
#undef VK_KEY
    KEY_COUNT
} key_code_t;

key_code_t key_conv_table[0xFF];

enum mouse_code_t {
    MOUSE_BTN_LEFT,
    MOUSE_BTN_MIDDLE,
    MOUSE_BTN_RIGHT,
    
    MOUSE_BTN_COUNT
} mouse_code_t;

typedef struct input_t {
    u8 key_state[KEY_COUNT];
    u8 mouse_state[MOUSE_BTN_COUNT];
} input_t;

#define is_key_down(__app, __key) \
__app->input.key_state[__key] & INPUT_STATE_DOWN

typedef struct app_t {
    renderer_t rb;
    asset_manager_t am;
    STACK(entity_t) *entities;
    STACK(physic_comp_t) *physics;
    
    input_t input;
} app_t;

#endif //APP_H