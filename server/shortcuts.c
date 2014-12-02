#include "../common/exceptions.h"
#include "../common/messages.h"
#include "shortcuts.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xutil.h>


Display *display;

void shortcuts_init() {
    display = XOpenDisplay(NULL);
    if (!display)
        throw_exc(ERR_DISPLAY);
}


struct ModifierRecord {
    const char *title;
    int value;
};

struct ModifierRecord modifier_records[] = {
    {"Shift", ShiftMask},
    {"CapsLock", LockMask},
    {"Ctrl", ControlMask},
    {"Alt", Mod1Mask},
    {"NumLock", Mod2Mask},
    {"AltLang", Mod3Mask},
    {"Kana", Mod4Mask},
    {"ScrollLock", Mod5Mask},
    {NULL, 0},
};

#define ERR_SHORTCUT_UNKNOWN_MODIFIER "Unknown modifier in shortcut \"%s\""
#define ERR_SHORTCUT_UNKNOWN_KEY "Unknown key \"%s\" in shortcut \"%s\""

struct Hotkey parse_hotkey(const char *str) {
    int word_begin = 0;
    struct Hotkey res;
    res.modifiers = 0;
    int i;
    for (i = 0; str[i]; i++)
        if (str[i] == '+') {
            int modifier_found = 0;
            int j;
            for (j = 0; modifier_records[j].title != NULL; j++)
                if (!strncmp(str + word_begin, modifier_records[j].title,
                        i - word_begin)) {
                    modifier_found = 1;
                    res.modifiers |= modifier_records[j].value;
                    break;
                }
            if (!modifier_found)
                throw_exc(ERR_SHORTCUT_UNKNOWN_MODIFIER, str);
            word_begin = i + 1;
        }
        
    res.keycode = XKeysymToKeycode(display, XStringToKeysym(str + word_begin));
    if (res.keycode == NoSymbol)
        throw_exc(ERR_SHORTCUT_UNKNOWN_KEY, str + word_begin, str);
    return res;
}


// Modifiers that can be set in addition to required
#define EXTRA_MODIFIERS (LockMask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

void grab_key(int keycode, unsigned modifiers,
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
}


void handle_shortcuts(const struct Shortcut shortcuts[]) {
    Window root = DefaultRootWindow(display);
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
    //XSync(display, 0);
}
