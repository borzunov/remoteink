#include "../common/exceptions.h"
#include "../common/messages.h"
#include "screen.h"
#include "shortcuts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xutil.h>
#include <xcb/xcb_keysyms.h>

xcb_connection_t *display;
xcb_key_symbols_t *key_symbols_table;
xcb_window_t root;

ExcCode shortcuts_init() {
	int default_screen_no;
	display = xcb_connect(NULL, &default_screen_no);
	if (xcb_connection_has_error(display))
		THROW(ERR_X_CONNECT);
	xcb_screen_t *screen;
	TRY(screen_of_display(display, default_screen_no, &screen));
	root = screen->root;
	key_symbols_table = xcb_key_symbols_alloc(display);
	if (key_symbols_table == NULL)
		THROW(ERR_X_REQUEST);
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
	int word_begin = 0;
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
		
	res->keycodes = xcb_key_symbols_get_keycode(key_symbols_table,
			XStringToKeysym(str + word_begin));
	if (res->keycodes == NULL)
		THROW(ERR_SHORTCUT_UNKNOWN_KEY, str + word_begin, str);
	return 0;
}


// Modifiers that can be set in addition to required
#define EXTRA_MODIFIERS (XCB_MOD_MASK_LOCK | \
		XCB_MOD_MASK_2 | XCB_MOD_MASK_3 | XCB_MOD_MASK_4 | XCB_MOD_MASK_5)

void grab_keycode(uint8_t owner_events,
		xcb_keycode_t keycode, unsigned modifiers,
		Window grab_window, uint8_t pointer_mode, uint8_t keyboard_mode) {
	unsigned rem_modifiers = EXTRA_MODIFIERS & ~modifiers;
	// Iterate through all submasks of rem_modifiers and grab key with
	// required modifiers and all variants of combinations of extra modifiers
	for (unsigned submask = rem_modifiers; ;
			submask = (submask - 1) & rem_modifiers) {
		xcb_grab_key(display, owner_events, grab_window,
				modifiers | submask, keycode, pointer_mode, keyboard_mode);
		// FIXME: handle errors?!
		if (!submask)
			break;
	}
}

void grab_hotkey(const struct Hotkey *hotkey) {
	for (int i = 0; hotkey->keycodes[i] != XCB_NO_SYMBOL; i++)
		grab_keycode(1, hotkey->keycodes[i], hotkey->modifiers, root,
				 XCB_GRAB_MODE_ASYNC,  XCB_GRAB_MODE_ASYNC);
}


void handle_shortcuts(const struct Shortcut shortcuts[]) {
	for (int i = 0; shortcuts[i].handler != NULL; i++)
		grab_hotkey(&shortcuts[i].hotkey);
	
	xcb_generic_event_t *event;
	while ((event = xcb_wait_for_event(display))) {
		if ((event->response_type & ~0x80) == XCB_KEY_RELEASE) {
			xcb_key_release_event_t *key_release_event =
					(xcb_key_release_event_t *) event;
			for (int i = 0; shortcuts[i].handler != NULL; i++) {
				unsigned modifiers = shortcuts[i].hotkey.modifiers;
				if (modifiers != key_release_event->state)
					continue;
				for (int j = 0;
						shortcuts[i].hotkey.keycodes[j] != XCB_NO_SYMBOL; j++)
					if (key_release_event->detail ==
							shortcuts[i].hotkey.keycodes[j]) {
						shortcuts[i].handler();
						break;
					}
			}
		}
		free(event);
	}
	// FIXME: an error occured, throw here?!
}
