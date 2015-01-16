#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <xcb/xcb.h>


extern ExcCode shortcuts_init(xcb_connection_t *cur_display,
		xcb_screen_t *cur_screen, xcb_window_t cur_root);

struct Hotkey {
	xcb_keycode_t *keycodes;
	unsigned modifiers;
};

extern ExcCode shortcuts_parse(const char *str, struct Hotkey *res);

struct Shortcut {
	struct Hotkey hotkey;
	void (*handler)();
};

extern ExcCode shortcuts_handle_start(const struct Shortcut shortcuts[]);


#endif
