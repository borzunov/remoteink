#include "../common/exceptions.h"
#include "../common/messages.h"
#include "shortcuts.h"

#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>


xcb_connection_t *display;

ExcCode shortcuts_init() {
	display = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(display))
		THROW(ERR_X_CONNECT);
	return 0;
}


struct ModifierRecord {
	const char *title;
	int value;
};

struct ModifierRecord modifier_records[] = {
	{"Shift", XCB_MOD_MASK_SHIFT},
	{"CapsLock", XCB_MOD_MASK_LOCK},
	{"Ctrl", XCB_MOD_MASK_CONTROL},
	{"Alt", XCB_MOD_MASK_1},
	{"NumLock", XCB_MOD_MASK_2},
	{"AltLang", XCB_MOD_MASK_3},
	{"Kana", XCB_MOD_MASK_4},
	{"ScrollLock", XCB_MOD_MASK_5},
	{NULL, 0},
};

#define ERR_SHORTCUT_UNKNOWN_MODIFIER "Unknown modifier in shortcut \"%s\""
#define ERR_SHORTCUT_UNKNOWN_KEY "Unknown key \"%s\" in shortcut \"%s\""

ExcCode parse_hotkey(const char *str, struct Hotkey *res) {
	/*int word_begin = 0;
	res->modifiers = 0;
	int i;
	for (i = 0; str[i]; i++)
		if (str[i] == '+') {
			int modifier_found = 0;
			int j;
			for (j = 0; modifier_records[j].title != NULL; j++)
				if (!strncmp(str + word_begin, modifier_records[j].title,
						i - word_begin)) {
					modifier_found = 1;
					res->modifiers |= modifier_records[j].value;
					break;
				}
			if (!modifier_found)
				THROW(ERR_SHORTCUT_UNKNOWN_MODIFIER, str);
			word_begin = i + 1;
		}
		
	res->keycode = XKeysymToKeycode(
			display, XStringToKeysym(str + word_begin));
	if (res->keycode == NoSymbol)
		THROW(ERR_SHORTCUT_UNKNOWN_KEY, str + word_begin, str);*/
	return 0;
}


// Modifiers that can be set in addition to required
#define EXTRA_MODIFIERS (XCB_MOD_MASK_LOCK, \
		XCB_MOD_MASK_2, XCB_MOD_MASK_3, XCB_MOD_MASK_4, XCB_MOD_MASK_5)

/*void grab_key(int keycode, unsigned modifiers,
		Window grab_window, Bool owner_events, int pointer_mode,
		int keyboard_mode) {
	unsigned rem_modifiers = EXTRA_MODIFIERS & ~modifiers;
	// Iterate through all submasks of rem_modifiers and grab key with
	// required modifiers and all variants of combinations of extra modifiers
	for (unsigned submask = rem_modifiers; ;
			submask = (submask - 1) & rem_modifiers) {
		XGrabKey(display, keycode, modifiers | submask,
				grab_window, owner_events, pointer_mode, keyboard_mode);
		
		if (!submask)
			break;
	}
}*/


void handle_shortcuts(const struct Shortcut shortcuts[]) {
	/*Window root = DefaultRootWindow(display);
	for (int i = 0; shortcuts[i].handler != NULL; i++) {
		grab_key(shortcuts[i].hotkey.keycode, shortcuts[i].hotkey.modifiers,
				root, 1, GrabModeAsync, GrabModeAsync);
	}

	for (;;) {
		XEvent event;
		XNextEvent(display, &event);
		if (event.type != KeyRelease)
			continue;

		for (int i = 0; shortcuts[i].handler != NULL; i++) {
			unsigned modifiers = shortcuts[i].hotkey.modifiers;
			if (
				event.xkey.keycode == shortcuts[i].hotkey.keycode &&
				(event.xkey.state & modifiers) == modifiers
			)
				shortcuts[i].handler();
		}
	}
	
	// FIXME: Correctly close grabs
	//XAllowEvents(display, AsyncKeyboard, CurrentTime);
	//XUngrabKey(display, key, modifiers, root);
	//XSync(display, 0);*/
}
