#include "client.h"
#include "main.h"
#include "options.h"

#include <inkview.h>

#define OPTIONS_FILE "/mnt/ext1/applications/inkmonitor.ini"

enum {STAGE_LABELS, STAGE_MONITOR} stage = STAGE_LABELS;

ifont *font_title, *font_comment;
#define BIG_INTERVAL 40
#define SMALL_INTERVAL 25

short label_x, label_y;

void clear_labels() {
    ClearScreen();
    label_x = 10;
    label_y = 10;
    SetFont(font_title, BLACK);
    
    DrawString(label_x, label_y, "InkMonitor v0.01 Alpha 4");
    label_y += BIG_INTERVAL;
    SetFont(font_comment, BLACK);
}

void show_intro() {
    clear_labels();
    
    DrawString(label_x, label_y,
            "Tool for using E-Ink reader as computer monitor");
    label_y += SMALL_INTERVAL;
    DrawString(label_x, label_y, "Copyright (c) 2013-2014 Alexander Borzunov");
    label_y += BIG_INTERVAL;
    
    DrawString(label_x, label_y, "Controls:");
    label_y += SMALL_INTERVAL;
    DrawString(label_x, label_y, "Left - exit");
    label_y += SMALL_INTERVAL;
    DrawString(label_x, label_y, "Right - start monitor");
    label_y += BIG_INTERVAL;
    
    static char message_buffer[256];
    sprintf(message_buffer, "Server: %s:%d", server_host, server_port);
    DrawString(label_x, label_y, message_buffer);
    label_y += SMALL_INTERVAL;
    
    FullUpdate();
}

inline void add_label(const char *message) {
    DrawString(label_x, label_y, message);
    label_y += SMALL_INTERVAL;
}

inline void show_error(const char *error) {
    stage = STAGE_LABELS;
    clear_labels();
    add_label("Error:");
    add_label(error);
    SoftUpdate();
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
        read_options(OPTIONS_FILE);
    
        screen_width = ScreenWidth();
        screen_height = ScreenHeight();
        
        SetAutoPowerOff(0);
        
        font_title = OpenFont("cour", 30, 1);
        font_comment = OpenFont("cour", 20, 1);
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
    }
    return 0;
}

int main() {
    InkViewMain(&main_handler);
    return 0;
}
