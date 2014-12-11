#include "../common/exceptions.h"
#include "../common/messages.h"
#include "screen.h"


Display *display;
Screen *screen;
Window root;

void screenshot_init(int *screen_width, int *screen_height) {
	display = XOpenDisplay(NULL);
	if (!display)
		throw_exc(ERR_DISPLAY);
	int screen_no = DefaultScreen(display);
	screen = ScreenOfDisplay(display, screen_no);
	*screen_width = screen->width;
	*screen_height = screen->height;

	Visual *visual = DefaultVisual(display, screen_no);
	Colormap colormap = DefaultColormap(display, screen_no);
	root = RootWindow(display, screen_no);

	imlib_context_set_display(display);
	imlib_context_set_visual(visual);
	imlib_context_set_colormap(colormap);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);
}

Imlib_Image screenshot_get(int x, int y, int width, int height) {
	return gib_imlib_create_image_from_drawable(root, 0,
			x, y, width, height, 1);
}


Window window_get_root() {
	return root;
}

Window window_get_focused() {
	// TODO:
	return root;
}

void window_get_geometry(Window window,
		int *left, int *top, int *width, int *height) {
	// TODO:
	*left = 0;
	*top = 0;
	*width = screen->width;
	*height = screen->height;
}
