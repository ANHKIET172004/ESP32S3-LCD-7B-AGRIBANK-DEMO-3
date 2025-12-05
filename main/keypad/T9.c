#include "T9.h"


extern keypad_context_t g_keypad;

const char *keypad_lower[10] = {
    " 0", ".,!?'\"1-(@*#", "abc2", "def3", "ghi4",
    "jkl5", "mno6", "pqrs7", "tuv8", "wxyz9"
};

const char *keypad_upper[10] = {
    " 0", ".,!?'\"1-(@*#", "ABC2", "DEF3", "GHI4",
    "JKL5", "MNO6", "PQRS7", "TUV8", "WXYZ9"
};


char get_char(char key, int *should_replace) {
    if (key < '0' || key > '9') {
        *should_replace = 0;
        return 0;
    }

    int digit = key - '0';
    const char **keypad_map = g_keypad.caps_lock ? keypad_upper : keypad_lower;
    const char *chars = keypad_map[digit];
    int len = strlen(chars);

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (key != g_keypad.last_key || (now - g_keypad.last_key_time) > T9_TIMEOUT_MS) {
        g_keypad.last_key = key;
        g_keypad.key_press_count = 1;
        g_keypad.last_key_time = now;
        *should_replace = 0;
        return chars[0];
    }

    g_keypad.key_press_count++;
    g_keypad.last_key_time = now;
    if (g_keypad.key_press_count > len) {
        g_keypad.key_press_count = 1;
    }

    *should_replace = 1;
    return chars[g_keypad.key_press_count - 1];
}