#include "../common/messages.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>


ExcCode screen_of_display(xcb_connection_t *c, int screen,
		xcb_screen_t **res) {
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(c));
	for (int i = 0; iter.rem; i++) {
		if (i == screen) {
			*res = iter.data;
			return 0;
		}
		xcb_screen_next(&iter);
	}
	THROW(ERR_X_REQUEST);
}


xcb_connection_t *display;
xcb_screen_t *screen;
xcb_window_t root;

ExcCode screenshot_init(int *screen_width, int *screen_height) {
	int default_screen_no;
	display = xcb_connect(NULL, &default_screen_no);
	if (xcb_connection_has_error(display))
		THROW(ERR_X_CONNECT);
	TRY(screen_of_display(display, default_screen_no, &screen));
	root = screen->root;
	*screen_width = screen->width_in_pixels;
	*screen_height = screen->height_in_pixels;
	return 0;
}

#define XCB_ALL_PLANES (~0)

ExcCode screenshot_get(int x, int y, int width, int height,
		unsigned **res_data) {
	// Note: *res_data will be set to pointer to a buffer that contains image
	//       pixels encoded as RGBX. This pointer should be freed by caller.
	xcb_get_image_cookie_t cookie = xcb_get_image(display,
			XCB_IMAGE_FORMAT_Z_PIXMAP, root, x, y, width, height,
			XCB_ALL_PLANES);
	xcb_get_image_reply_t *reply = xcb_get_image_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	int res_data_length = xcb_get_image_data_length(reply);
	*res_data = malloc(res_data_length);
	memcpy(*res_data, (unsigned *) xcb_get_image_data(reply), res_data_length);
	free(reply);
	return 0;
}


ExcCode window_get_root(xcb_window_t *res) {
	*res = root;
	return 0;
}

ExcCode window_get_focused(xcb_window_t *res) {
	// TODO:
	*res = root;
	return 0;
}

ExcCode window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height) {
	// TODO:
	*left = 0;
	*top = 0;
	*width = screen->width_in_pixels;
	*height = screen->height_in_pixels;
	return 0;
}
