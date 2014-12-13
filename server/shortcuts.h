#ifndef HOTKEYS_H
#define HOTKEYS_H


ExcCode shortcuts_init();


struct Hotkey {
	int keycode;
	unsigned modifiers;
};

ExcCode parse_hotkey(const char *str, struct Hotkey *res);

struct Shortcut {
	struct Hotkey hotkey;
	void (*handler)();
};

void handle_shortcuts(const struct Shortcut shortcuts[]);


#endif
