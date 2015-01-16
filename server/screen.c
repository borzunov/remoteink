#include "../common/messages.h"
#include "../common/utils.h"
#include "options.h"
#include "screen.h"

#include <stdlib.h>
#include <string.h>
#include <xcb/xfixes.h>


xcb_connection_t *display;
xcb_screen_t *screen;
xcb_window_t root;

#define ERR_CURSOR "Cursor capturing isn't supported"

ExcCode screen_cursor_init() {
	xcb_xfixes_query_version_cookie_t cookie =
			xcb_xfixes_query_version(display,
			XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
	xcb_xfixes_query_version_reply_t *reply =
			xcb_xfixes_query_version_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_CURSOR);
	return 0;
}

ExcCode screen_init(xcb_connection_t *cur_display,
		xcb_screen_t *cur_screen, xcb_window_t cur_root) {
	display = cur_display;
	screen = cur_screen;
	root = cur_root;
	
	screen_cursor_init();
	return 0;
}

ExcCode screen_cursor_blend(int x, int y, Imlib_Image image) {
	xcb_xfixes_get_cursor_image_cookie_t cookie =
			xcb_xfixes_get_cursor_image(display);
	xcb_xfixes_get_cursor_image_reply_t *reply =
			xcb_xfixes_get_cursor_image_reply(display, cookie, NULL);
	if (reply == NULL)
		return 0;
	unsigned *cursor_data = xcb_xfixes_get_cursor_image_cursor_image(reply);
	if (cursor_data == NULL)
		return 0;
	Imlib_Image cursor = imlib_create_image_using_data(
			reply->width, reply->height,
			cursor_data);
	if (cursor == NULL)
		THROW(ERR_IMAGE);
	imlib_context_set_image(cursor);
	imlib_image_set_has_alpha(1);
	
	imlib_context_set_image(image);
	imlib_blend_image_onto_image(cursor, 0, 0, 0, reply->width, reply->height,
			reply->x - reply->xhot - x, reply->y - reply->yhot - y,
			reply->width, reply->height);
	
	imlib_context_set_image(cursor);
	imlib_free_image_and_decache();
	free(reply);
	return 0;
}

#define XCB_ALL_PLANES (~0)

ExcCode screen_shot(int x, int y, int width, int height,
		Imlib_Image *res) {
	int x1 = x + width;
	int y1 = y + height;
	x = MAX(x, 0);
	y = MAX(y, 0);
	x1 = MIN(x1, screen->width_in_pixels);
	y1 = MIN(y1, screen->height_in_pixels);
	width = x1 - x;
	height = y1 - y;
	
	xcb_get_image_cookie_t cookie = xcb_get_image(display,
			XCB_IMAGE_FORMAT_Z_PIXMAP, root, x, y, width, height,
			XCB_ALL_PLANES);
	xcb_get_image_reply_t *reply = xcb_get_image_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	*res = imlib_create_image_using_copied_data(width, height,
			(unsigned *) xcb_get_image_data(reply));
	free(reply);
	if (*res == NULL)
		THROW(ERR_IMAGE);
		
	if (cursor_capturing_enabled)
		screen_cursor_blend(x, y, *res);
	return 0;
}


ExcCode create_atom(const char *name, uint8_t only_if_exists,
		xcb_atom_t *res) {
	xcb_intern_atom_cookie_t cookie =
			xcb_intern_atom(display, only_if_exists, strlen(name), name);
	xcb_intern_atom_reply_t *reply =
			xcb_intern_atom_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	*res = reply->atom;
	free(reply);
	return 0;
}

void get_net_frame_window(xcb_window_t window, xcb_window_t *res) {
	xcb_atom_t net_frame_window;
	if (create_atom("_NET_FRAME_WINDOW", 1, &net_frame_window)) {
		*res = window;
		return;
	}
	xcb_get_property_cookie_t cookie =
			xcb_get_property(display, 0, window,
			net_frame_window, XCB_ATOM_ANY, 0, 1);
	xcb_get_property_reply_t *reply =
			xcb_get_property_reply(display, cookie, NULL);
	if (
		reply == NULL ||
		reply->type == XCB_ATOM_NONE || reply->format != 32 ||
		xcb_get_property_value_length(reply) != sizeof (xcb_window_t)
	) {
		*res = window;
		return;
	}
	*res = *(xcb_window_t *) xcb_get_property_value(reply);
	free(reply);
}

void find_toplevel_window(xcb_window_t window, xcb_window_t *res) {
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
}


ExcCode screen_cursor_get_position(int *x, int *y, int *same_screen) {
	xcb_query_pointer_cookie_t cookie =
			xcb_query_pointer(display, root);
	xcb_query_pointer_reply_t *reply =
			xcb_query_pointer_reply(display, cookie, NULL);
	if (reply == NULL)
		THROW(ERR_X_REQUEST);
	*x = reply->root_x;
	*y = reply->root_y;
	*same_screen = reply->same_screen;
	free(reply);
	return 0;
}

ExcCode screen_cursor_set_position(int x, int y) {
	xcb_void_cookie_t cookie =
			xcb_warp_pointer(display, XCB_NONE, root, 0, 0, 0, 0, x, y);
	if (xcb_request_check(display, cookie) != NULL)
		THROW(ERR_X_REQUEST);
	if (xcb_flush(display) <= 0)
		THROW(ERR_X_REQUEST);
	return 0;
}


void screen_get_root(xcb_window_t *res) {
	*res = root;
}

ExcCode screen_get_focused(xcb_window_t *res) {
	xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(display);
	xcb_get_input_focus_reply_t *reply =
			xcb_get_input_focus_reply(display, cookie, NULL);
	if (reply == NULL) {
		*res = root;
		return 0;
	}
	*res = reply->focus;
	free(reply);
	return 0;
}

ExcCode window_get_real_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height) {
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

ExcCode screen_window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height) {
	find_toplevel_window(window, &window);
	get_net_frame_window(window, &window);
	TRY(window_get_real_geometry(window, left, top, width, height));
	return 0;
}


void find_window_by_property(xcb_window_t window, xcb_atom_t property,
		xcb_window_t *res) {
	*res = XCB_WINDOW_NONE;
	
	xcb_get_property_cookie_t get_property_cookie =
			xcb_get_property(display, 0, window, property, XCB_ATOM_ANY, 0, 0);
	xcb_get_property_reply_t *get_property_reply =
			xcb_get_property_reply(display, get_property_cookie, NULL);
	if (get_property_reply != NULL) {
		if (get_property_reply->type != XCB_ATOM_NONE) {
			*res = window;
			free(get_property_reply);
			return;
		}
		free(get_property_reply);
	}
	
	xcb_query_tree_cookie_t query_tree_cookie =
			xcb_query_tree(display, window);
	xcb_query_tree_reply_t *query_tree_reply =
			xcb_query_tree_reply(display, query_tree_cookie, NULL);
	if (query_tree_reply == NULL)
		return;
	int length = xcb_query_tree_children_length(query_tree_reply);
	xcb_window_t *children = xcb_query_tree_children(query_tree_reply);
	for (int i = 0; i < length; i++) {
		find_window_by_property(children[i], property, res);
		if (*res != XCB_WINDOW_NONE)
			break;
	}
	free(query_tree_reply);
}

void get_client_window(xcb_window_t window, xcb_window_t *res) {
	xcb_atom_t wm_state;
	if (create_atom("WM_STATE", 1, &wm_state)) {
		*res = window;
		return;
	}
	find_window_by_property(window, wm_state, res);
	if (*res == XCB_WINDOW_NONE)
		*res = window;
}

ExcCode screen_window_unmaximize(xcb_window_t window) {
	xcb_atom_t net_wm_state,
			net_wm_state_maximized_horz, net_wm_state_maximized_vert;
	TRY(create_atom("_NET_WM_STATE", 0, &net_wm_state));
	TRY(create_atom("_NET_WM_STATE_MAXIMIZED_HORZ", 0,
			&net_wm_state_maximized_horz));
	TRY(create_atom("_NET_WM_STATE_MAXIMIZED_VERT", 0,
			&net_wm_state_maximized_vert));
	xcb_client_message_event_t event;
	event.response_type = XCB_CLIENT_MESSAGE;
	event.format = 32;
	event.sequence = 0;
	event.window = window;
	event.type = net_wm_state;
	event.data.data32[0] = 0; // 0 - disable, 1 - enable, 2 - toggle
	event.data.data32[1] = net_wm_state_maximized_horz;
	event.data.data32[2] = net_wm_state_maximized_vert;
	event.data.data32[3] = 0;
	event.data.data32[4] = 0;
	xcb_void_cookie_t cookie = xcb_send_event_checked(display, 0, root,
			XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(const char*) &event);
	if (xcb_request_check(display, cookie) != NULL)
		THROW(ERR_X_REQUEST);
	if (xcb_flush(display) <= 0)
		THROW(ERR_X_REQUEST);
	return 0;
}


ExcCode screen_window_resize(xcb_window_t window, int width, int height) {
	xcb_window_t client_window;
	find_toplevel_window(window, &client_window);
	get_client_window(client_window, &client_window);
	
	TRY(screen_window_unmaximize(client_window));
	
	int coord, prev_width, prev_height;
	TRY(screen_window_get_geometry(window, &coord, &coord,
			&prev_width, &prev_height));
	int prev_client_window_width, prev_client_window_height;
	TRY(window_get_real_geometry(client_window, &coord, &coord,
			&prev_client_window_width, &prev_client_window_height));
	uint32_t values[2] = {
		width - (prev_width - prev_client_window_width),
		height - (prev_height - prev_client_window_height)
	};
    xcb_void_cookie_t cookie = xcb_configure_window_checked(display,
			client_window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
			values);
	if (xcb_request_check(display, cookie) != NULL)
		THROW(ERR_X_REQUEST);
    if (xcb_flush(display) <= 0)
		THROW(ERR_X_REQUEST);
	return 0;
}
