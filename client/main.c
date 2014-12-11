#include "../common/exceptions.h"
#include "../common/messages.h"
#include "client.h"
#include "main.h"
#include "options.h"
#include "ui.h"

#if defined(__i386__)
#define OPTIONS_DIR "."
#else
#define OPTIONS_DIR GAMEPATH
#endif
#define OPTIONS_FILE OPTIONS_DIR "/inkmonitor.ini"

#define TEXT_TITLE "InkMonitor v0.01 Alpha 4"

enum Stage {STAGE_INTRO, STAGE_MONITOR};

enum Stage stage = STAGE_INTRO;

#define MESSAGE_MSECS 1000

ifont *font_title, *font_label, *font_caption;
int font_caption_offset;
#define LINE_SPACING 3
#define PARAGRAPH_EXTRA_SPACING 12

struct UIControl *controls[CONTROLS_MAX_COUNT];
int controls_top = 0;

#define SCREEN_PADDING 10
short label_y;

int pointer_need_repaint = 1;

void change_option_handler(char *buffer) {
	save_config(OPTIONS_FILE);
	ui_repaint(controls, controls_top);
}

#define HOST_MAXLEN 16

void edit_host_handler() {
	pointer_need_repaint = 0;
	OpenKeyboard("Host:", server_host, HOST_MAXLEN,
			KBD_NORMAL, change_option_handler);
}

#define BUFFER_SIZE 256
char server_port_buffer[BUFFER_SIZE];
#define PORT_MAXLEN 5

void change_port_handler(char *buffer) {
	int incorrect = 0;
	if (buffer) {
		int number;
		if (
			sscanf(server_port_buffer, "%d", &number) == 1 &&
			PORT_MIN <= number && number <= PORT_MAX
		) {
			server_port = number;
		} else {
			incorrect = 1;
			sprintf(server_port_buffer, "%d", server_port);
		}
	}
	change_option_handler(buffer);
	if (incorrect) {
		char message_buffer[256];
		sprintf(
			message_buffer,
			"Port number should be in the interval from %d to %d",
			PORT_MIN, PORT_MAX
		);
		show_error(message_buffer);
	}
}

void edit_port_handler() {
	pointer_need_repaint = 0;
	sprintf(server_port_buffer, "%d", server_port);
	OpenKeyboard("Port:", server_port_buffer, PORT_MAXLEN,
			KBD_NUMERIC, change_port_handler);
}

void clear_labels() {
	label_y = SCREEN_PADDING;
	
	int i;
	for (i = controls_top - 1; i != -1; i--)
		ui_control_destroy(controls[i]);
	controls[0] = ui_label_create(
		SCREEN_PADDING, screen_width - SCREEN_PADDING, label_y,
		UI_ALIGN_CENTER,
		TEXT_TITLE, font_title, BLACK,
		1
	);
	controls_top = 1;
	
	label_y += font_title->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
}

inline void add_label(const char *message) {
	controls[controls_top++] = ui_label_create(
		SCREEN_PADDING, screen_width - SCREEN_PADDING, label_y, UI_ALIGN_LEFT,
		message, font_label, BLACK,
		1
	);
	
	label_y += font_label->height + LINE_SPACING;
}

#define FIELD_TEXT_MARGIN_LEFT 70

const char *edit_text = "Edit";

inline void add_field(const char *caption, const char *text,
		void (*edit_handler)()) {
	SetFont(font_caption, BLACK);
	int edit_text_width = StringWidth(edit_text);
	int button_right = screen_width - SCREEN_PADDING;
			
	controls[controls_top++] = ui_label_create(
		SCREEN_PADDING, FIELD_TEXT_MARGIN_LEFT,
		label_y + font_caption_offset, UI_ALIGN_LEFT,
		caption, font_caption, BLACK,
		1
	); // Caption
	controls[controls_top++] = ui_label_create(
		FIELD_TEXT_MARGIN_LEFT, screen_width - FIELD_TEXT_MARGIN_LEFT,
		label_y, UI_ALIGN_LEFT,
		text, font_label, BLACK,
		1
	); // Content
	controls[controls_top++] = ui_button_create(
		button_right - 2 * UI_BUTTON_PADDING - edit_text_width, button_right,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		edit_text, font_caption, BLACK,
		edit_handler,
		1
	); // "Edit" button
	
	label_y += font_label->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
}

pthread_t client_thread;

void exec_connect() {
	stage = STAGE_MONITOR;
	SetAutoPowerOff(0);
	
	int res;
	if (pthread_create(&client_thread, NULL, client_connect, &res) != 0)
		show_error("Failed to create client thread");
}

int schedule_connect = 0;

void connect_handler() {
	schedule_connect = 1;
}

const char *connect_text = "Connect";
const char *quit_text = "Quit";

void show_intro() {
	stage = STAGE_INTRO;
	SetAutoPowerOff(1);
	
	clear_labels();
	add_label("Tool for using E-Ink reader as computer monitor");
	add_label("Copyright (c) 2013-2014 Alexander Borzunov");
	label_y += PARAGRAPH_EXTRA_SPACING * 2;
	
	add_label("	Controls");
	label_y += PARAGRAPH_EXTRA_SPACING;
	SetFont(font_caption, BLACK);
	int left = SCREEN_PADDING;
	controls[controls_top++] = ui_button_create(
		left, left + StringWidth(connect_text) + 2 * UI_BUTTON_PADDING,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		connect_text, font_caption, BLACK,
		connect_handler,
		1
	);
	SetFont(font_caption, BLACK);
	int right = screen_width - SCREEN_PADDING;
	controls[controls_top++] = ui_button_create(
		right - 2 * UI_BUTTON_PADDING - StringWidth(quit_text), right,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		quit_text, font_caption, BLACK,
		CloseApp,
		1
	);
	label_y += font_label->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
	// Another button
	label_y += PARAGRAPH_EXTRA_SPACING;
	
	add_label("	Settings");
	label_y += PARAGRAPH_EXTRA_SPACING;
	add_field("Host:", server_host, edit_host_handler);
	sprintf(server_port_buffer, "%d", server_port);
	add_field("Port:", server_port_buffer, edit_port_handler);
	
	ui_repaint(controls, controls_top);
}

void show_error(const char *error) {
	HideHourglass();
	show_intro();
	
	Message(ICON_ERROR, "Error", error, MESSAGE_MSECS);
}

inline void show_conn_error(const char *message) {
	show_error(message);
	pthread_exit(NULL);
}

const char *recv_error = "Failed to read data";
const char *send_error = "Failed to write data";

void query_network() {
	if (!(QueryNetwork() & NET_CONNECTED)) {
		NetConnect(NULL);
		
		// Network selection dialog can ruin the image on the screen. Let's
		// wait until it disappears.
		sleep(1);
	}
}

void stop_monitor_handler() {
	client_process = 0;
	client_shutdown();
}

int main_handler(int type, int par1, int par2) {
	switch (type) {
	case EVT_INIT:
		set_except(NULL);
		load_config(OPTIONS_FILE);
	
		screen_width = ScreenWidth();
		screen_height = ScreenHeight();
		
		font_title = OpenFont("cour", 30, 1);
		font_label = OpenFont("cour", 20, 1);
		font_caption = OpenFont("cour", 17, 1);
		font_caption_offset = (font_label->height - font_caption->height) / 2;
		break;
	case EVT_SHOW:
		if (stage == STAGE_INTRO)
			show_intro();
		break;
	case EVT_KEYPRESS:
		switch (par1) {
		case KEY_LEFT:
		case KEY_PREV:
			if (stage == STAGE_INTRO)
				CloseApp();
			else
			if (stage == STAGE_MONITOR)
				stop_monitor_handler();
			break;
		case KEY_RIGHT:
		case KEY_NEXT:
			if (stage == STAGE_INTRO)
				connect_handler();
			break;
		}
		break;
	case EVT_POINTERDOWN:
	case EVT_POINTERUP:
		if (stage == STAGE_INTRO) {
			ui_pointer(controls, controls_top, type, par1, par2);
			if (pointer_need_repaint) {
				ui_repaint(controls, controls_top);
			} else
				pointer_need_repaint = 1;
		}
		break;
	}
	if (schedule_connect) {
		exec_connect();
		schedule_connect = 0;
	}
	return 0;
}

int main() {
	InkViewMain(&main_handler);
	return 0;
}
