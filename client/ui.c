#include "ui.h"


void ui_control_destroy(struct UIControl *control) {
	free(control->data);
	free(control);
}


void ui_repaint(struct UIControl *controls[], int count) {
	ClearScreen();
	int i;
	for (i = 0; i < count; i++)
		if (controls[i]->visible && controls[i]->on_repaint)
			controls[i]->on_repaint(controls[i]->data);
	SoftUpdate();
}

void ui_pointer(struct UIControl *controls[], int count,
		int action, int x, int y) {
	int i;
	for (i = count - 1; i != -1; i--)
		if (controls[i]->visible && controls[i]->on_pointer)
			controls[i]->on_pointer(controls[i]->data, action, x, y);
}


void ui_label_repaint(struct UILabel *label) {
	SetFont(label->font, label->color);
	int need_width = StringWidth(label->text);
	
	int text_left;
	switch (label->align) {
	case UI_ALIGN_LEFT:
		text_left = label->left;
		break;
	case UI_ALIGN_CENTER:
		text_left = (label->left + label->right - need_width) / 2;
		break;
	}
	DrawString(text_left, label->top, label->text);
}

struct UIControl *ui_label_create(
	int left, int right, int top, enum UIAlign align,
	const char *text, ifont *font, int color,
	int visible
) {
	struct UILabel *label = (struct UILabel *) malloc(sizeof (struct UILabel));
	label->left = left;
	label->right = right;
	label->top = top;
	label->align = align;
	label->text = text;
	label->font = font;
	label->color = color;
	
	struct UIControl *control = (struct UIControl *) malloc(
			sizeof (struct UIControl));
	control->data = (void *) label;
	control->visible = visible;
	control->on_repaint = (void (*)(void *)) ui_label_repaint;
	control->on_pointer = 0;
	return control;
}


void ui_button_repaint(struct UIButton *button) {
	int height = button->font->height + 2 * UI_BUTTON_PADDING;
	if (button->pressed)
		FillArea(
			button->left, button->top, button->right - button->left, height,
			LGRAY
		);
	DrawRect(
		button->left, button->top, button->right - button->left, height,
		button->color
	);
	
	SetFont(button->font, button->color);
	int need_width = StringWidth(button->text);
	
	int text_left;
	switch (button->align) {
	case UI_ALIGN_LEFT:
		text_left = button->left + UI_BUTTON_PADDING;
		break;
	case UI_ALIGN_CENTER:
		text_left = (button->left + button->right - need_width) / 2;
		break;
	}
	DrawString(text_left, button->top + UI_BUTTON_PADDING, button->text);
}

void ui_button_pointer(struct UIButton *button,
		int action, int x, int y) {
	int target = (
		button->left <= x && x < button->right &&
		button->top <= y &&
		y < button->top + button->font->height + 2 * UI_BUTTON_PADDING
	);
	
	if (action == EVT_POINTERUP) {
		if (button->pressed && target)
			button->click_handler();
		button->pressed = 0;
	} else
	if (action == EVT_POINTERDOWN) {
		if (target)
			button->pressed = 1;
	}
}

struct UIControl *ui_button_create(
	int left, int right, int top, enum UIAlign align,
	const char *text, ifont *font, int color,
	void (*click_handler)(),
	int visible
) {
	struct UIButton *button =
			(struct UIButton *) malloc(sizeof (struct UIButton));
	button->left = left;
	button->right = right;
	button->top = top;
	button->align = align;
	button->text = text;
	button->font = font;
	button->color = color;
	button->click_handler = click_handler;
	button->pressed = 0;
	
	struct UIControl *control = (struct UIControl *) malloc(
			sizeof (struct UIControl));
	control->data = (void *) button;
	control->visible = visible;
	control->on_repaint = (void (*)(void *)) ui_button_repaint;
	control->on_pointer = (void (*)(void *, int, int, int)) ui_button_pointer;
	return control;
}
