#include "wrappers.h"
#include <string.h>
#include "tilel.h"

xcb_window_t wrapper_active_window()
{
	xcb_get_property_cookie_t c =
		xcb_ewmh_get_active_window_unchecked(&ewmh, screen);
	xcb_window_t f = 0;
	xcb_ewmh_get_active_window_reply(&ewmh, c, &f, NULL);
	return f;
}

uint32_t wrapper_current_desktop()
{
	xcb_get_property_cookie_t c =
		xcb_ewmh_get_current_desktop_unchecked(&ewmh, screen);
	uint32_t desktop = 0;
	xcb_ewmh_get_current_desktop_reply(&ewmh, c, &desktop, NULL);
	return desktop;
}

uint32_t wrapper_client_list(xcb_window_t **output)
{
	xcb_get_property_cookie_t c =
		xcb_ewmh_get_client_list_unchecked(&ewmh, screen);
	xcb_ewmh_get_windows_reply_t r;
	if (!xcb_ewmh_get_client_list_reply(&ewmh, c, &r, NULL))
		return 0;

	uint32_t len = r.windows_len;
	uint32_t bytes = len * sizeof(xcb_window_t);

	*output = malloc(bytes);
	memcpy(*output, r.windows, bytes);
	xcb_ewmh_get_windows_reply_wipe(&r);

	return len;
}

uint32_t wrapper_wm_desktop(xcb_window_t w)
{
	xcb_get_property_cookie_t c = xcb_ewmh_get_wm_desktop_unchecked(&ewmh, w);
	uint32_t desktop = 0;
	xcb_ewmh_get_wm_desktop_reply(&ewmh, c, &desktop, NULL);
	return desktop;
}

uint32_t wrapper_wm_window_type(xcb_window_t w, xcb_atom_t **output)
{
	xcb_get_property_cookie_t c =
		xcb_ewmh_get_wm_window_type_unchecked(&ewmh, w);
	xcb_ewmh_get_atoms_reply_t r;
	if (!xcb_ewmh_get_wm_window_type_reply(&ewmh, c, &r, NULL))
		return 0;

	uint32_t len = r.atoms_len;
	uint32_t bytes = len * sizeof(xcb_atom_t);

	*output = malloc(bytes);
	memcpy(*output, r.atoms, bytes);
	xcb_ewmh_get_atoms_reply_wipe(&r);

	return len;
}

void wrapper_change_active(xcb_window_t w)
{
	xcb_window_t f = wrapper_active_window();
	xcb_ewmh_request_change_active_window(&ewmh, screen, w,
			XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER, XCB_CURRENT_TIME, f);
}

static void wrapper_frame_extents(xcb_window_t w, uint32_t output[4])
{
	xcb_get_property_cookie_t c = 
		xcb_ewmh_get_frame_extents_unchecked(&ewmh, w);
	xcb_ewmh_get_extents_reply_t r;
	xcb_ewmh_get_frame_extents_reply(&ewmh, c, &r, NULL);
	output[0] = r.left;
	output[1] = r.right;
	output[2] = r.top;
	output[3] = r.bottom;
}

void wrapper_move_resize(xcb_window_t w, uint32_t extents[4])
{
	uint32_t frame[4];
	wrapper_frame_extents(w, frame);

	extents[0] += screen_x;
	extents[1] += screen_y;
	extents[2] -= (frame[0] + frame[1]);
	extents[3] -= (frame[2] + frame[3]);

	xcb_ewmh_request_moveresize_window(&ewmh, screen, w, 0,
			XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL,
			XCB_EWMH_MOVERESIZE_WINDOW_X |
			XCB_EWMH_MOVERESIZE_WINDOW_Y |
			XCB_EWMH_MOVERESIZE_WINDOW_WIDTH |
			XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT,
			extents[0], extents[1], extents[2], extents[3]);
}