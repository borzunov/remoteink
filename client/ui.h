#ifndef UI_H
#define UI_H


#include <inkview.h>


struct UIControl {
    void *data;
    int visible;
    
    void (*on_repaint)(void *data);
    void (*on_pointer)(void *data, int action, int x, int y);
};

void ui_control_destroy(struct UIControl *control);

void ui_repaint(struct UIControl *controls[], int count);
void ui_pointer(struct UIControl *controls[], int count,
        int action, int x, int y);


enum UIAlign {UI_ALIGN_LEFT, UI_ALIGN_CENTER};


struct UILabel {
    int left, right, top;
    enum UIAlign align;
    
    const char *text;
    ifont *font;
    int color;
};

void ui_label_repaint(struct UILabel *label);

struct UIControl *ui_label_create(
    int left, int right, int top, enum UIAlign align,
    const char *text, ifont *font, int color,
    int visible
);


struct UIButton {
    int left, right, top;
    enum UIAlign align;
    
    const char *text;
    ifont *font;
    int color;
    
    void (*click_handler)();
    
    int pressed;
};

#define UI_BUTTON_PADDING 5

void ui_button_repaint(struct UIButton *button);
void ui_button_pointer(struct UIButton *button,
        int action, int x, int y);

struct UIControl *ui_button_create(
    int left, int right, int top, enum UIAlign align,
    const char *text, ifont *font, int color,
    void (*click_handler)(),
    int visible
);


#endif
