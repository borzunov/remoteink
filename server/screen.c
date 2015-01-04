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


ExcCode get_net_frame_window(xcb_window_t window, xcb_window_t *res) {
	const char *atom_name = "_NET_FRAME_WINDOW";
	xcb_intern_atom_cookie_t intern_atom_cookie =
			xcb_intern_atom(display, 1, strlen(atom_name), atom_name);
	xcb_intern_atom_reply_t *intern_atom_reply =
			xcb_intern_atom_reply(display, intern_atom_cookie, NULL);
	if (intern_atom_reply == NULL) {
		*res = window;
		return 0;
	}
	xcb_atom_t net_frame = intern_atom_reply->atom;
	free(intern_atom_reply);
	
	xcb_get_property_cookie_t get_property_cookie =
			xcb_get_property(display, 0, window,
			net_frame, XCB_ATOM_ANY, 0, 1);
	xcb_get_property_reply_t *get_property_reply =
			xcb_get_property_reply(display, get_property_cookie, NULL);
	if (
		get_property_reply == NULL ||
		get_property_reply->type == XCB_ATOM_NONE ||
		get_property_reply->format != sizeof (xcb_window_t) ||
		xcb_get_property_value_length(get_property_reply) !=
				sizeof (xcb_window_t) / 8
	) {
		*res = window;
		return 0;
	}
	printf("xcb_get_property_value_length(get_property_reply) = %d\n", xcb_get_property_value_length(get_property_reply)); //
	*res = *(xcb_window_t *) xcb_get_property_value(get_property_reply);
	free(get_property_reply);
	return 0;
}

ExcCode find_toplevel_window(xcb_window_t window, xcb_window_t *res) {
	for (;;) {
		xcb_query_tree_cookie_t cookie = xcb_query_tree(display, window);
		xcb_query_tree_reply_t *reply =
				xcb_query_tree_reply(display, cookie, NULL);
		if (reply == NULL)
			break;
		if (reply->parent == XCB_WINDOW_NONE || reply->parent == reply->root) {
			free(reply);
			break;
		}
		window = reply->parent;
		free(reply);
	}
	*res = window;
	return 0;
}


ExcCode window_get_root(xcb_window_t *res) {
	*res = root;
	return 0;
}

ExcCode window_get_focused(xcb_window_t *res) {
	xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(display);
	xcb_get_input_focus_reply_t *reply =
			xcb_get_input_focus_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	*res = reply->focus;
	free(reply);
	return 0;
}

ExcCode window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height) {
	TRY(find_toplevel_window(window, &window));
	TRY(get_net_frame_window(window, &window));
	xcb_get_geometry_cookie_t cookie =
			xcb_get_geometry(display, window);
	xcb_get_geometry_reply_t *reply =
			xcb_get_geometry_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	*left = reply->x;
	*top = reply->y;
	*width = reply->width;
	*height = reply->height;
	free(reply);
	return 0;
}
