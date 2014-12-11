#ifndef HOTKEYS_H
#define HOTKEYS_H


void shortcuts_init();


struct Hotkey {
	int keycode;
	unsigned modifiers;
};

struct Hotkey parse_hotkey(const char *str);

struct Shortcut {
	struct Hotkey hotkey;
	void (*handler)();
};

void handle_shortcuts(const struct Shortcut shortcuts[]);


#endif
