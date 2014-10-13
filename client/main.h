#ifndef MAIN_H
#define MAIN_H


#define CONTROLS_MAX_COUNT 32
extern struct UIControl *controls[CONTROLS_MAX_COUNT];
extern int controls_top;

void clear_labels();
void add_label(const char *message);

void show_intro();

void show_error(const char *error);
void show_conn_error(const char *message);
void show_client_error();

extern const char *recv_error, *send_error;

void query_network();


#endif
