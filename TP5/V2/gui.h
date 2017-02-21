#ifndef GUI_H
#define GUI_H

void window_init (GtkApplication* app, gpointer user_data);

void editing_init (Mydata *data);

void layout_init (gpointer user_data);

void status_init (gpointer user_data);

void win_scale_init (Mydata *my);

#endif// GUI_H
