#include "../common/exceptions.h"
#include "../common/messages.h"
#include "screen.h"


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


int screen_left = 0;
int screen_top = 0;

#define MOVE_STEP 10

void move_up_handler() {
    screen_top -= MOVE_STEP;
}

void move_down_handler() {
    screen_top += MOVE_STEP;
}

void move_left_handler() {
    screen_left -= MOVE_STEP;
}

void move_right_handler() {
    screen_left += MOVE_STEP;
}
