#include "../common/exceptions.h"
#include "../common/messages.h"
#include "../common/utils.h"
#include "client.h"
#include "options.h"
#include "ui.h"

#define TEXT_TITLE "RemoteInk v0.3"

#define CONFIG_BASENAME "remoteink.ini"

#define CONFIG_FILENAME_SIZE 256
char config_filename[CONFIG_FILENAME_SIZE];

enum Stage {STAGE_INTRO, STAGE_MONITOR};

enum Stage stage = STAGE_INTRO;

ifont *font_title, *font_label, *font_caption;
int font_caption_offset;
#define LINE_SPACING 3
#define PARAGRAPH_EXTRA_SPACING 12

#define CONTROLS_MAX_COUNT 32
struct UIControl *controls[CONTROLS_MAX_COUNT];
int controls_top = 0;

struct UIButton *buttons_tab_order[CONTROLS_MAX_COUNT];
int buttons_tab_order_top = 0;
int buttons_tab_cur = -1;

#define SCREEN_PADDING 10
short label_y;

int pointer_need_repaint = 1;

void show_error(const char *error);
void show_intro();

void change_option_handler(char *buffer) {
	options_config_save(config_filename);
	ui_repaint(controls, controls_top);
}

#define HOST_MAXLEN 16

void edit_host_handler() {
	pointer_need_repaint = 0;
	OpenKeyboard("Host:", server_host, HOST_MAXLEN,
			KBD_NORMAL, change_option_handler);
}

#define SERVER_PORT_BUFFER_SIZE 256
char server_port_buffer[SERVER_PORT_BUFFER_SIZE];
#define PORT_MAXLEN 5

void submit_port() {
	snprintf(server_port_buffer, SERVER_PORT_BUFFER_SIZE, "%d", server_port);
}

void change_port_handler(char *buffer) {
	int incorrect = 0;
	if (buffer && parse_int(
			"Port", server_port_buffer, PORT_MIN, PORT_MAX, &server_port)) {
		incorrect = 1;
		submit_port();
	}
	change_option_handler(buffer);
	if (incorrect)
		show_error(exc_message);
}

void edit_port_handler() {
	pointer_need_repaint = 0;
	OpenKeyboard("Port:", server_port_buffer, PORT_MAXLEN,
			KBD_NUMERIC, change_port_handler);
}

char password_hidden[PASSWORD_SIZE];

#define PASSWORD_HIDING_CHAR '*'

void submit_password() {
	int i;
	for (i = 0; password[i]; i++)
		password_hidden[i] = PASSWORD_HIDING_CHAR;
	password_hidden[i] = 0;
}

void change_password_handler(char *buffer) {
	if (buffer)
		submit_password();
	change_option_handler(buffer);
}

void edit_password_handler() {
	pointer_need_repaint = 0;
	submit_port();
	OpenKeyboard("Password:", password, PORT_MAXLEN,
			KBD_PASSWORD, change_password_handler);
}

const char *orientation_cur_caption;

int client_width, client_height;

void update_orientation() {
	SetOrientation(orientation);
	client_width = ScreenWidth();
	client_height = ScreenHeight();
}

void submit_orientation() {
	orientation_cur_caption = orientation_captions[orientation];
}

void switch_orientation_handler() {
	orientation = !orientation;
	submit_orientation();
	update_orientation();
	
	options_config_save(config_filename);
	show_intro();
}

void clear_labels() {
	label_y = SCREEN_PADDING;
	
	int i;
	for (i = controls_top - 1; i != -1; i--)
		ui_control_destroy(controls[i]);
	controls[0] = ui_label_create(
		SCREEN_PADDING, client_width - SCREEN_PADDING, label_y,
		UI_ALIGN_CENTER,
		TEXT_TITLE, font_title, BLACK,
		1
	);
	controls_top = 1;
	buttons_tab_order_top = 0;
	
	label_y += font_title->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
}

void add_label(const char *message) {
	controls[controls_top++] = ui_label_create(
		SCREEN_PADDING, client_width - SCREEN_PADDING, label_y, UI_ALIGN_LEFT,
		message, font_label, BLACK,
		1
	);
	
	label_y += font_label->height + LINE_SPACING;
}

#define FIELD_TEXT_MARGIN_LEFT 120

void add_field(const char *label_caption, const char *text,
		const char *button_caption, void (*button_handler)()) {
	SetFont(font_caption, BLACK);
	int button_caption_width = StringWidth(button_caption);
	int button_right = client_width - SCREEN_PADDING;
			
	controls[controls_top++] = ui_label_create(
		SCREEN_PADDING, FIELD_TEXT_MARGIN_LEFT,
		label_y + font_caption_offset, UI_ALIGN_LEFT,
		label_caption, font_caption, BLACK,
		1
	); // Caption
	controls[controls_top++] = ui_label_create(
		FIELD_TEXT_MARGIN_LEFT, client_width - FIELD_TEXT_MARGIN_LEFT,
		label_y, UI_ALIGN_LEFT,
		text, font_label, BLACK,
		1
	); // Content
	struct UIControl *button_control = ui_button_create(
		button_right - 2 * UI_BUTTON_PADDING - button_caption_width,
		button_right,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		button_caption, font_caption, BLACK,
		button_handler,
		1
	);
	controls[controls_top++] = button_control; // Button
	buttons_tab_order[buttons_tab_order_top++] = button_control->data;
	
	label_y += font_label->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
}

#define AGREEMENT_CONTENT \
		"This program is experimental. Reader's screens are not intended " \
		"to update so frequently, so using the program can lead " \
		"to malfunction of the screen or reduce its lifetime. Developer " \
		"of the program is not responsible for possible damage to your " \
		"device."

void agreement_accept_handler(int button) {
	if (button != 1) {
		show_error("You can't use program without accepting the agreement");
		CloseApp();
		return;
	}
	agreement_accepted = 1;
	options_config_save(config_filename);
}

pthread_t client_thread;

void *start_client_connect(void *arg) {
	if (client_connect(client_width, client_height))
		show_error(exc_message);
	else
		show_intro();
	return NULL;
}

ExcCode exec_connect() {
	stage = STAGE_MONITOR;
	SetAutoPowerOff(0);
	
	int res;
	if (pthread_create(&client_thread, NULL, start_client_connect, &res) != 0)
		PANIC(ERR_THREAD_CREATE);
	return 0;
}

int schedule_connect = 0;

void connect_handler() {
	schedule_connect = 1;
}

const char *connect_text = "Connect";
const char *quit_text = "Quit";

const char *edit_text = "Edit";

void show_intro() {
	stage = STAGE_INTRO;
	SetAutoPowerOff(1);
	
	clear_labels();
	add_label("Tool for using E-Ink reader as computer monitor");
	add_label("Copyright © 2013-2018 Alexander Borzunov");
	label_y += PARAGRAPH_EXTRA_SPACING * 2;
	
	add_label("    Controls");
	label_y += PARAGRAPH_EXTRA_SPACING;
	
	SetFont(font_caption, BLACK);
	int left = SCREEN_PADDING;
	struct UIControl *button_control = ui_button_create(
		left, left + StringWidth(connect_text) + 2 * UI_BUTTON_PADDING,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		connect_text, font_caption, BLACK,
		connect_handler,
		1
	);
	controls[controls_top++] = button_control;
	buttons_tab_order[buttons_tab_order_top++] = button_control->data;
	
	int right = client_width - SCREEN_PADDING;
	button_control = ui_button_create(
		right - 2 * UI_BUTTON_PADDING - StringWidth(quit_text), right,
		label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
		quit_text, font_caption, BLACK,
		CloseApp,
		1
	);
	controls[controls_top++] = button_control;
	buttons_tab_order[buttons_tab_order_top++] = button_control->data;
	label_y += font_label->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
	label_y += PARAGRAPH_EXTRA_SPACING;
	
	add_label("    Settings");
	label_y += PARAGRAPH_EXTRA_SPACING;
	add_field("Host:", server_host, edit_text, edit_host_handler);
	submit_port();
	add_field("Port:", server_port_buffer, edit_text, edit_port_handler);
	submit_password();
	add_field("Password:", password_hidden, edit_text, edit_password_handler);
	submit_orientation();
	add_field("Orientation:", orientation_cur_caption,
			"Switch", switch_orientation_handler);
	
	if (buttons_tab_cur != -1 && buttons_tab_cur < buttons_tab_order_top)
		buttons_tab_order[buttons_tab_cur]->focused = 1;
	
	ui_repaint(controls, controls_top);
	
	if (!agreement_accepted)
		Dialog(ICON_WARNING, "Warning", AGREEMENT_CONTENT,
				"Accept", "Reject", agreement_accept_handler);
}

void show_error(const char *error) {
	HideHourglass();
	show_intro();
	
	Message(ICON_ERROR, "Error", error, MESSAGE_MSECS);
}

void stop_monitor_handler() {
	client_shutdown();
}

int main_handler(int type, int par1, int par2) {
	switch (type) {
		case EVT_INIT:
			get_default_config_path(CONFIG_BASENAME,
					config_filename, CONFIG_FILENAME_SIZE);
			options_config_load(config_filename);

			SetPanelType(0);
			update_orientation();
			
			font_title = OpenFont("cour", 30, 1);
			font_label = OpenFont("cour", 20, 1);
			font_caption = OpenFont("cour", 17, 1);
			font_caption_offset =
					(font_label->height - font_caption->height) / 2;
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
				
				case KEY_DOWN:
					if (stage == STAGE_INTRO && buttons_tab_order_top) {
						if (buttons_tab_cur == -1)
							buttons_tab_cur = 0;
						else {
							buttons_tab_order[buttons_tab_cur]->focused = 0;
							
							buttons_tab_cur++;
							if (buttons_tab_cur >= buttons_tab_order_top)
								buttons_tab_cur = 0;
						}
						buttons_tab_order[buttons_tab_cur]->focused = 1;
						ui_repaint(controls, controls_top);
					}
					break;
				case KEY_UP:
					if (stage == STAGE_INTRO && buttons_tab_order_top) {
						if (buttons_tab_cur == -1)
							buttons_tab_cur = buttons_tab_order_top - 1;
						else {
							buttons_tab_order[buttons_tab_cur]->focused = 0;
							
							buttons_tab_cur--;
							if (buttons_tab_cur < 0)
								buttons_tab_cur = buttons_tab_order_top - 1;
						}
						buttons_tab_order[buttons_tab_cur]->focused = 1;
						ui_repaint(controls, controls_top);
					}
					break;
				case KEY_OK:
					if (stage == STAGE_INTRO && buttons_tab_cur != -1 &&
							buttons_tab_cur < buttons_tab_order_top)
						buttons_tab_order[buttons_tab_cur]->click_handler();
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
		if (exec_connect())
			show_error(exc_message);
		schedule_connect = 0;
	}
	return 0;
}

int main() {
	InkViewMain(&main_handler);
	return 0;
}
