#ifndef MAIN_H
#define MAIN_H


extern void clear_labels();
extern void add_label(const char *message);
extern void show_error(const char *error);
extern void show_conn_error(const char *message);
extern void show_client_error();

extern void query_network();


#endif
