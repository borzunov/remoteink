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

enum {STAGE_LABELS, STAGE_MONITOR} stage = STAGE_LABELS;

#define MESSAGE_MSECS 1000

ifont *font_title, *font_label, *font_caption;
#define LINE_SPACING 3
#define PARAGRAPH_EXTRA_SPACING 12

struct UIControl *controls[CONTROLS_MAX_COUNT];
int controls_top = 0;

#define SCREEN_PADDING 10
short label_y;

int pointer_need_repaint = 1;

void change_option_handler(char *buffer) {
    save_options(OPTIONS_FILE);
    ui_repaint(controls, controls_top);
}

#define HOST_MAXLEN 16

void edit_host_handler() {
    pointer_need_repaint = 0;
    OpenKeyboard("Host:", server_host, HOST_MAXLEN,
            KBD_NORMAL, change_option_handler);
}

char server_port_buffer[OPTION_SIZE];

#define PORT_MAXLEN 5
#define PORT_MIN 0
#define PORT_MAX 65536

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
            "Port number must be in the interval from %d to %d!",
            PORT_MIN, PORT_MAX
        );
        Message(ICON_ERROR, "Error", message_buffer, MESSAGE_MSECS);
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
    int font_caption_offset = (font_label->height - font_caption->height) / 2;
    int edit_text_width = StringWidth(edit_text);
    int button_left = screen_width - SCREEN_PADDING -
            2 * UI_BUTTON_PADDING - edit_text_width;
            
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
        button_left, screen_width - SCREEN_PADDING,
        label_y - UI_BUTTON_PADDING + font_caption_offset, UI_ALIGN_LEFT,
        edit_text, font_caption, BLACK,
        edit_handler,
        1
    ); // "Edit" button
    
    label_y += font_label->height + LINE_SPACING + PARAGRAPH_EXTRA_SPACING;
}

void show_intro() {
    clear_labels();
    add_label("Tool for using E-Ink reader as computer monitor");
    add_label("Copyright (c) 2013-2014 Alexander Borzunov");
    label_y += PARAGRAPH_EXTRA_SPACING * 2;
    
    add_label("    Controls");
    label_y += PARAGRAPH_EXTRA_SPACING;
    add_label("Left - exit");
    add_label("Right - start monitor");
    label_y += PARAGRAPH_EXTRA_SPACING * 2;
    
    add_label("    Settings");
    label_y += PARAGRAPH_EXTRA_SPACING;
    add_field("Host:", server_host, edit_host_handler);
    sprintf(server_port_buffer, "%d", server_port);
    add_field("Port:", server_port_buffer, edit_port_handler);
    
    ui_repaint(controls, controls_top);
}

inline void show_error(const char *error) {
    stage = STAGE_LABELS;
    clear_labels();
    add_label("Error:");
    add_label(error);
    
    ui_repaint(controls, controls_top);
}

inline void show_conn_error(const char *message) {
    show_error(message == NULL ? "Connection failure" : message);
    pthread_exit(NULL);
}

inline void show_client_error() {
    show_error("Connection closed");
    pthread_exit(NULL);
}

void query_network() {
    if (!(QueryNetwork() & NET_CONNECTED)) {
        NetConnect(NULL);
        
        // Network selection dialog can ruin the image on the screen. Let's
        // wait until it disappears.
        sleep(1);
    }
}

int main_handler(int type, int par1, int par2) {
    pthread_t thread;
    int res;
    switch (type) {
    case EVT_INIT:
        load_options(OPTIONS_FILE);
    
        screen_width = ScreenWidth();
        screen_height = ScreenHeight();
        
        SetAutoPowerOff(0);
        
        font_title = OpenFont("cour", 30, 1);
        font_label = OpenFont("cour", 20, 1);
        font_caption = OpenFont("cour", 17, 1);
        break;
    case EVT_SHOW:
        if (stage == STAGE_LABELS)
            show_intro();
        break;
    case EVT_KEYPRESS:
        switch (par1) {
        case KEY_LEFT:
        case KEY_PREV:
            CloseApp();
            break;
        case KEY_RIGHT:
        case KEY_NEXT:
            if (stage == STAGE_LABELS) {
                stage = STAGE_MONITOR;
                if (pthread_create(&thread, NULL, &client_connect, &res) != 0)
                    show_error("Failed to create client thread");
            }
            break;
        }
        break;
    case EVT_POINTERDOWN:
    case EVT_POINTERUP:
        if (stage == STAGE_LABELS) {
            ui_pointer(controls, controls_top, type, par1, par2);
            if (pointer_need_repaint) {
                ui_repaint(controls, controls_top);
            } else
                pointer_need_repaint = 1;
        }
        break;
    }
    return 0;
}

int main() {
    InkViewMain(&main_handler);
    return 0;
}
