#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <X11/Xutil.h>


void shortcuts_init();


struct Hotkey {
    int keycode;
    unsigned modifiers;
};

struct Hotkey parse_hotkey(const char *str);


// Modifiers that can be set in addition to required:
//     CapsLock, NumLock, AltLang, Kana, ScrollLock
#define EXTRA_MODIFIERS (LockMask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)


struct Shortcut {
    struct Hotkey hotkey;
    void (*handler)();
};

void handle_shortcuts(const struct Shortcut shortcuts[]);


#endif
