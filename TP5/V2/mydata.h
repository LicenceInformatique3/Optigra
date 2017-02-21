#ifndef MYDATA_H
#define MYDATA_H

enum const_edit{EDIT_NONE,EDIT_ADD_CURVE,EDIT_MOVE_CURVE,EDIT_REMOVE_CURVE,EDIT_ADD_CONTROL,EDIT_MOVE_CONTROL,EDIT_REMOVE_CONTROL,EDIT_LAST};

typedef struct {
    GtkWidget *window;
    GtkWidget *status;
    GtkWidget *vbox;
    GtkWidget *win_scale;
    GtkWidget *scale_horizon;
    GtkWidget *area;
    GtkWidget *scroll;
    GtkWidget *menu_bar;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget * add_curve;
	GtkWidget * move_curve;
	GtkWidget * remove_curve;
	GtkWidget * add_control;
	GtkWidget * move_control;
	GtkWidget * remove_control;
	GtkWidget *vertical;
	GtkWidget *edit_radios[EDIT_LAST];
	GdkPixbuf * pixbuf, *pixbuf2;
	double click_x,click_y,last_x, last_y,rotate_angle,scale_horizon_value;
    int win_width,win_height;
    int show_edit;
    int edit_mode;
	int click_n;
    int clip_image;
    unsigned int magic;
    char * current_folder;
    char *title;
    Curve_infos curve_infos;
} Mydata;

Mydata *get_mydata (gpointer data);

void init_mydata(Mydata *my);

void set_edit_mode (Mydata *data, int mode);

#endif /* MYDATA_H */
