#include "../common/exceptions.h"
#include "../common/messages.h"
#include "screenshot.h"


void screenshot_init(Screen **screen, Window *root) {
    Display *display = XOpenDisplay(NULL);
    if (!display)
        throw_exc(ERR_DISPLAY);
    int screen_no = DefaultScreen(display);
    *screen = ScreenOfDisplay(display, screen_no);

    Visual *visual = DefaultVisual(display, screen_no);
    Colormap colormap = DefaultColormap(display, screen_no);
    *root = RootWindow(display, screen_no);

    imlib_context_set_display(display);
    imlib_context_set_visual(visual);
    imlib_context_set_colormap(colormap);
    imlib_context_set_color_modifier(NULL);
    imlib_context_set_operation(IMLIB_OP_COPY);
}

Imlib_Image screenshot_get(Window root, int x, int y, int width, int height) {
    return gib_imlib_create_image_from_drawable(root, 0,
            x, y, width, height, 1);
}
