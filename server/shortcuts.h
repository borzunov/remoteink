#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <xcb/xcb.h>


extern ExcCode shortcuts_init();

struct Hotkey {
	xcb_keycode_t *keycodes;
	unsigned modifiers;
};

extern ExcCode parse_hotkey(const char *str, struct Hotkey *res);

struct Shortcut {
	struct Hotkey hotkey;
	void (*handler)();
};

extern ExcCode handle_shortcuts(const struct Shortcut shortcuts[]);


#endif
