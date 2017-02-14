#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MYDATA_MAGIC 0x46EA7E05
#define CONTROL_MAX 100
#define CURVE_MAX 200

enum const_edit{EDIT_NONE,EDIT_ADD_CURVE,EDIT_MOVE_CURVE,EDIT_REMOVE_CURVE,EDIT_ADD_CONTROL,EDIT_MOVE_CONTROL,EDIT_REMOVE_CONTROL,EDIT_LAST};

typedef struct{
    double x;
    double y;
} Control;

typedef struct{
    int control_count;
    Control controls[CONTROL_MAX];
} Curve;

typedef struct{
    int curve_count;
    Curve curves[CURVE_MAX];
} Curve_list;

typedef struct{
    Curve_list curve_list;
    int current_curve;
    int current_control;
} Curve_infos;

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

void init_curve_infos(Curve_infos *ci){
    ci->curve_list.curve_count = 0;
    ci->current_control = -1;
    ci->current_curve = -1;
}

void init_mydata(Mydata *my){
    my->title = "TP1 en GTK";
    my->win_width = 500;
    my->win_height = 400;
    my->magic = MYDATA_MAGIC;
    my->current_folder = NULL;
    my->pixbuf = NULL;
    my->scale_horizon_value = 1.0;
    my->scale_horizon = NULL;
    my->pixbuf2 = NULL;
    my->rotate_angle = 0.0;
    my->click_x = 0.0f;
    my->click_y = 0.0f;
    my->click_n = 0;
    my->last_x = 0.0f;
    my->last_y = 0.0f;    
    my->clip_image = FALSE;
    my->show_edit = FALSE;
    my->edit_mode = EDIT_ADD_CURVE;
    init_curve_infos(&my->curve_infos);
}

// Cette fonction permet de tester si le data que l’on a recuperé dans
// une callback contient bien my ; sinon, cela veut dire que :
//  - soit on a oublié de transmettre my dans g_signal_connect,
//  - soit on s’est trompé dans le nombre de paramètres de la callback.
Mydata *get_mydata (gpointer data)
{
    if (data == NULL) {
        fprintf (stderr, "get_mydata: NULL data\n"); return NULL;
    }
    if (((Mydata *)data)->magic != MYDATA_MAGIC) {
        fprintf (stderr, "get_mydata: bad magic number\n"); return NULL;
    }
    return data;
}
// Mise à jour de la barre de statut

void set_status (Mydata *data, const char *msg){
    Mydata *my = get_mydata(data);
    gtk_statusbar_pop (GTK_STATUSBAR (my->status), 0); 
    gtk_statusbar_push (GTK_STATUSBAR (my->status), 0, msg);
}

// Gestion de l'édition : Curves / Controls

void set_edit_mode (Mydata *data, int mode) {
    Mydata *my = get_mydata(data);
    if (mode > EDIT_NONE || mode < EDIT_LAST) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (my->edit_radios[mode-1]), TRUE);
    }
}

int add_curve (Curve_infos *ci) {
    if (ci->curve_list.curve_count == CURVE_MAX) {
        ci->current_curve = -1;
        return -1;
    }
    int n = ci->curve_list.curve_count;
    ci->curve_list.curve_count++;
    ci->curve_list.curves[n].control_count = 0;
    ci->current_curve = n;
    ci->current_control = -1;
    return n;
}

int add_control (Curve_infos *ci, double x, double y) {
    int n = ci->current_curve;    
    Curve *curve = &ci->curve_list.curves[n];
    if (n < 0 || n > ci->curve_list.curve_count) return -1;
    if (curve->control_count >= CONTROL_MAX) return -1;
    int k = curve->control_count;
    curve->control_count++;
    curve->controls[k].x = x;
    curve->controls[k].y = y;
    ci->current_control = k;
    return k;
}

void draw_curves (cairo_t *cr, Curve_infos *ci) {
    Curve *curve;
    for (int i = 0; i < ci->curve_list.curve_count; ++i)    {
        curve = &ci->curve_list.curves[i];
        cairo_move_to (cr, curve->controls[0].x, curve->controls[0].y);
        for (int j = 0; j < curve->control_count; ++j) {
            cairo_set_line_width (cr, 3);
            if (i == ci->current_curve)
                cairo_set_source_rgb (cr, 1.0, 1.0, 0);
            else
                cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
            cairo_line_to (cr, curve->controls[j].x, curve->controls[j].y);        
        }
        cairo_stroke (cr);    
    }


    for (int i = 0; i < ci->curve_list.curve_count; ++i)    {
        curve = &ci->curve_list.curves[i];
        for (int j = 0; j < curve->control_count; ++j) {
            cairo_set_line_width (cr, 3);
            if (i == ci->current_curve && j == ci->current_control)
                cairo_set_source_rgb (cr, 1.0, 0, 0);
            else
                cairo_set_source_rgb (cr, 0, 0, 1.0);
            cairo_rectangle (cr, curve->controls[j].x - 3.0, 
                             curve->controls[j].y - 3.0, 6.0, 6.0);
            cairo_stroke (cr);            
        }

    }
}

int find_control (Curve_infos * ci, double x, double y) {    
    Curve *curve;
    for (int i = 0; i <= ci->curve_list.curve_count; ++i)    {
        curve = &ci->curve_list.curves[i];
        for (int j = 0; j < curve->control_count; ++j) {
            double dx = curve->controls[j].x - x;
            double dy = curve->controls[j].y - y;
            if (dx * dx + dy * dy <= 5 * 5) {
                ci->current_curve = i;
                ci->current_control = j;
                printf("Curve : %d\n", i);
                return 0;
            }
        }
    }
    ci->current_curve = -1;
    ci->current_control = -1;
    return -1;    
}

int move_control (Curve_infos * ci, double dx, double dy) {
    int n = ci->current_curve;
    int k = ci->current_control;
    if ((n < 0 || n > ci->curve_list.curve_count - 1) ||
       (k < 0 || k > ci->curve_list.curves[n].control_count - 1))
        return -1;
    Curve *curve = &ci->curve_list.curves[n];
    curve->controls[k].x = curve->controls[k].x + dx;
    curve->controls[k].y = curve->controls[k].y + dy;
    return 0;
}

int move_curve (Curve_infos * ci, double dx, double dy) {
    int n = ci->current_curve;
    if (n < 0 || n > ci->curve_list.curve_count - 1)
        return -1;
    Curve *curve = &ci->curve_list.curves[n];
    for (int i = 0; i < curve->control_count; ++i) {    
        curve->controls[i].x = curve->controls[i].x + dx;
        curve->controls[i].y = curve->controls[i].y + dy;            
    }
    return 0;
}

int remove_curve (Curve_infos * ci) {
    int n = ci->current_curve;
    printf("current_curve : %d, curve_count : %d\n", n, ci->curve_list.curve_count);
    if (n < 0 || n > ci->curve_list.curve_count - 1) {
        return -1;
    }
    memmove (ci->curve_list.curves+n, ci->curve_list.curves+n+1, 
             sizeof(Curve)*(ci->curve_list.curve_count-1-n));
    ci->curve_list.curve_count--;
    ci->current_curve = -1;
    return 0;
}

int remove_control (Curve_infos *ci) {
    int n = ci->current_curve;
    int k = ci->current_control;
    Curve *curve = &ci->curve_list.curves[n];
    if (k < 0 || k > curve->control_count - 1) {
        printf("Suppression échouée\n");
        return -1;
    }    
    memmove (curve->controls+k, curve->controls+k+1, 
             sizeof(Control)*(curve->control_count-1-k));
    curve->control_count--;
    if (curve->control_count < 1) {
        remove_curve(ci);
    }
    ci->current_control = -1;
    printf("Suppression réussie\n");
    return 0;
}


// Gestion DrawingArea : refresh, scale, rotation

void refresh_area (GtkWidget *area){
    GdkWindow *win = gtk_widget_get_window (area);
    if (win == NULL) return;
    gdk_window_invalidate_rect (win, NULL, FALSE);
}

void apply_image_transforms (Mydata *data) {    
    Mydata *my = get_mydata(data);    
    g_clear_object(&my->pixbuf2);
    if (my->pixbuf == NULL) return;
    printf ("pixbuf : %d x %d, angle %f\n", gdk_pixbuf_get_width (my->pixbuf), gdk_pixbuf_get_height (my->pixbuf), my->rotate_angle);
    GdkPixbuf *tmp = gdk_pixbuf_rotate_simple (my->pixbuf, my->rotate_angle);                
    int width = gdk_pixbuf_get_width (tmp);
    int height = gdk_pixbuf_get_height (tmp);    
    printf ("tmp : %d x %d\n", width, height);
    
    my->pixbuf2 = gdk_pixbuf_scale_simple (tmp, width * my->scale_horizon_value, height * my->scale_horizon_value, GDK_INTERP_BILINEAR);
    printf ("pixbuf2 : %d x %d\n", gdk_pixbuf_get_width (my->pixbuf2), gdk_pixbuf_get_height (my->pixbuf2));
    
    g_object_unref (tmp);    
}

void update_area_with_transforms (Mydata *data) {
    Mydata *my = get_mydata(data);
    apply_image_transforms (my);
    if (my->pixbuf2 != NULL) {
        gtk_widget_set_size_request (my->area, gdk_pixbuf_get_width (my->pixbuf2), gdk_pixbuf_get_height (my->pixbuf2));    
        refresh_area (my->area);    
    }
}

// Callbacks menus / boutons

void on_item_load_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Load Image",
                                          GTK_WINDOW(my->window),
                                          action,
                                          "Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    if (my->current_folder != NULL) 
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), my->current_folder);
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        set_status(my, "Loading image ...");
        g_clear_object(&my->pixbuf);
        my->pixbuf = gdk_pixbuf_new_from_file(filename, NULL);       
        if (my->pixbuf == NULL) { 
            set_status(my, "Loading failed : not an image.");
            //gtk_image_set_from_icon_name (GTK_IMAGE (my->image1), "image-missing",
            //                              GTK_ICON_SIZE_DIALOG);
        }
        else {
            char str[80];
            sprintf(str, "Loading success : image %dx%d.", gdk_pixbuf_get_width(my->pixbuf),
                                                         gdk_pixbuf_get_height(my->pixbuf));
            
            set_status(my, str);
            //gtk_widget_set_size_request (my->area, gdk_pixbuf_get_width (my->pixbuf), gdk_pixbuf_get_height (my->pixbuf));
            my->scale_horizon_value = 1.0;
            my->rotate_angle = 0.0;
            apply_image_transforms (my);
        }
        my->current_folder = NULL;
        my->current_folder = gtk_file_chooser_get_current_folder(chooser);    
        g_free(filename);    
    }

    gtk_widget_destroy (dialog);
    
    printf ("Button clicked in window ’%s’\n", my->title);
}

void on_item_quit_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    printf ("Closing window and app\n");
    gtk_widget_destroy(my->window);
}

void on_item_about_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    char *auteurs[] = {"Ahmed Belamri <ahmed.belamri.1@etu.univ-amu.fr>", NULL};
    gtk_show_about_dialog (NULL, "program-name", my->title, "version", 
                           "2.4", "website", "http://j.mp/optigra", 
                           "authors", auteurs, "logo-icon-name", 
                           "face-wink",NULL);    
    printf ("About\n");
}

void on_item_color_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    gint res = 0;
    printf ("Color\n");
    GtkWidget *color_chooser;
    color_chooser = gtk_color_chooser_dialog_new ("Background color", NULL);
    res = gtk_dialog_run(GTK_DIALOG (color_chooser));
    if (res == GTK_RESPONSE_OK) { 
        GdkRGBA bg_color;
        gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (color_chooser), &bg_color);
        gchar *c;
        c = gdk_rgba_to_string (&bg_color);
        char s[1000];
        sprintf(s, "Selected bg color : %s", c);
        set_status (my, s);
        gtk_widget_override_background_color(my->area, GTK_STATE_FLAG_NORMAL, &bg_color);
        g_free(c);
    }
    gtk_widget_destroy(color_chooser);
}

void on_item_rotate_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    if (my->pixbuf != NULL){
        my->rotate_angle = my->rotate_angle + 90;
        if (my->rotate_angle >= 360) 
            my->rotate_angle = my->rotate_angle - 360;
        update_area_with_transforms (my);
        
        set_status(my, "Image rotated.");
    }
    else 
        set_status(my, "No image to rotate.");
}

void on_item_scale_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);     
    gtk_range_set_value(GTK_RANGE(my->scale_horizon),my->scale_horizon_value);
    gtk_window_present (GTK_WINDOW (my->win_scale));
}

void on_item_clip_activate (GtkCheckMenuItem *widget, gpointer data){
    Mydata *my = get_mydata(data);    
    
    my->clip_image = gtk_check_menu_item_get_active (widget);
    if (my->clip_image == TRUE)
        set_status(my, "Clipping is on");
    else
        set_status(my, "Clipping is off");
        
    refresh_area (my->area);
}

void on_item_edit_activate (GtkCheckMenuItem *widget, gpointer data){
    Mydata *my = get_mydata(data);    
    
    my->show_edit = gtk_check_menu_item_get_active (widget);
    if (my->show_edit == TRUE) {
        set_status(my, "Editing is on");
        gtk_widget_show (my->frame);
    }
    else {
        set_status(my, "Editing is off");
        gtk_widget_hide (my->frame);
    }
        
    refresh_area (my->area);
}

void on_scale_horizon_value_changed (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
       if (gtk_range_get_value(GTK_RANGE(widget)) == my->scale_horizon_value) return;
    if (my->pixbuf2 != NULL) {
        my->scale_horizon_value = gtk_range_get_value(GTK_RANGE(widget));    
        update_area_with_transforms (my);    
    }
    
    else 
        set_status(my, "No image to scale.");
}

void on_radio_toggled (GtkWidget *widget, gpointer data) {
    Mydata *my = get_mydata(data);
    gint mode= GPOINTER_TO_INT(g_object_get_data (G_OBJECT(widget), "mode"));
    my->edit_mode = mode;
}

// Callbacks area

gboolean on_area_key_press (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    GdkEventKey *evk = &event->key;
    printf ("%s: GDK_KEY_%s\n",    __func__, gdk_keyval_name(evk->keyval));
    switch (evk->keyval) {
        case GDK_KEY_q : on_item_quit_activate (area, data); break;
        case GDK_KEY_a : set_edit_mode (my, EDIT_ADD_CURVE); break;
        case GDK_KEY_z : set_edit_mode (my, EDIT_MOVE_CURVE); break;
        case GDK_KEY_e : set_edit_mode (my, EDIT_REMOVE_CURVE); break;
        case GDK_KEY_r : set_edit_mode (my, EDIT_ADD_CONTROL); break;
        case GDK_KEY_t : set_edit_mode (my, EDIT_MOVE_CONTROL); break;
        case GDK_KEY_y : set_edit_mode (my, EDIT_REMOVE_CONTROL); break;
    }
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_key_release (GtkWidget *area, GdkEvent *event, gpointer data){
    GdkEventKey *evk = &event->key;
    printf ("%s: GDK_KEY_%s\n",    __func__, gdk_keyval_name(evk->keyval));
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_button_press (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);     
    int n;
    GdkEventButton *evb = &event->button;
    printf ("%s: %d %.1f %.1f\n", __func__, evb->button, evb->x, evb->y);
    my->click_y = evb->y;
    my->click_x = evb->x;
    my->click_n = evb->button;
    if (my->click_n == 1 && my->show_edit == TRUE) {
        switch (my->edit_mode) {
            case EDIT_ADD_CURVE : 
                n = add_curve (&my->curve_infos);
                if(n < 0) break;
                set_edit_mode (my, EDIT_ADD_CONTROL);
                add_control (&my->curve_infos, my->click_x, my->click_y);
                refresh_area (my->area);
                break;
            case EDIT_MOVE_CURVE : 
                find_control (&my->curve_infos, my->click_x, my->click_y);                
                refresh_area (my->area); 
                break;
            case EDIT_REMOVE_CURVE : 
                n = find_control (&my->curve_infos, my->click_x, my->click_y);
                if (n == 0) remove_curve (&my->curve_infos);
                refresh_area (my->area); 
                break;
            case EDIT_ADD_CONTROL : 
                add_control (&my->curve_infos, my->click_x, my->click_y);
                refresh_area (my->area); 
                break;
            case EDIT_MOVE_CONTROL : 
                find_control (&my->curve_infos, my->click_x, my->click_y);
                refresh_area (my->area); 
                break;
            case EDIT_REMOVE_CONTROL : 
                n = find_control (&my->curve_infos, my->click_x, my->click_y);
                if (n == 0) remove_control (&my->curve_infos);            
                refresh_area (my->area); 
                break;
        }
    }
    refresh_area(my->area);
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_button_release (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);     
    GdkEventButton *evb = &event->button;
    printf ("%s: %d %.1f %.1f\n", __func__, evb->button, evb->x, evb->y);
    my->click_n = 0;
    refresh_area(my->area);
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_motion_notify (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);     
    GdkEventMotion *evm = &event->motion;    
    my->last_y = my->click_y;
    my->last_x = my->click_x ;
    my->click_y = evm->y ;
    my->click_x = evm->x ;
    if (my->click_n == 1 && my->show_edit == TRUE) {
        switch (my->edit_mode) {
            case EDIT_ADD_CURVE : 
                refresh_area (my->area);
                break;
            case EDIT_MOVE_CURVE : 
                move_curve (&my->curve_infos, my->click_x - my->last_x, 
                              my->click_y - my->last_y);
                refresh_area (my->area); 
                break;
            case EDIT_REMOVE_CURVE : refresh_area (my->area); break;
            case EDIT_ADD_CONTROL : 
                refresh_area (my->area); 
                break;
            case EDIT_MOVE_CONTROL : 
                move_control (&my->curve_infos, my->click_x - my->last_x, 
                              my->click_y - my->last_y);
                refresh_area (my->area); 
                break;
            case EDIT_REMOVE_CONTROL : refresh_area (my->area); break;
        }
    }
    refresh_area(my->area);
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_enter_notify (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    gtk_widget_grab_focus (my->area);
    GdkEventCrossing *evc = &event->crossing;
    printf ("%s: %.1f %.1f\n", __func__, evc->x, evc->y);
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_leave_notify (GtkWidget *area, GdkEvent *event, gpointer data){
    GdkEventCrossing *evc = &event->crossing;
    printf ("%s: %.1f %.1f\n", __func__, evc->x, evc->y);
    return TRUE;  //  ́ev ́enement trait ́e
}

gboolean on_area_draw (GtkWidget *area, cairo_t *cr, gpointer data){    
    Mydata *my = get_mydata(data);
    if(my->pixbuf2 != NULL){
        int pix_width = gdk_pixbuf_get_width(my->pixbuf2);
        int pix_height = gdk_pixbuf_get_height(my->pixbuf2);
        gdk_cairo_set_source_pixbuf(cr,my->pixbuf2,0,0);
        if (my->clip_image == FALSE) {
            cairo_rectangle (cr, 0.0, 0.0, pix_width, pix_height);
            cairo_fill (cr);
        }
    }

    draw_curves (cr, &my->curve_infos);
    return TRUE;
}

// Initialisations graphiques

void window_init (GtkApplication* app, gpointer user_data){
    Mydata *my = get_mydata(user_data);
    
    my->window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (my->window), my->title);
    gtk_window_set_default_size (GTK_WINDOW (my->window), 
                                 my->win_width, my->win_height);
}

void editing_init (Mydata *data) {
    Mydata *my = get_mydata(data);

    GtkWidget *vbox2 =     gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);

    my->frame = gtk_frame_new ("Editing");

    char *noms[6] = {"Add curve", "Move curve", "Remove curve", "Add control", 
                    "Move control", "Remove control"};
    
    for (int i = 0; i < 6; i++) {
        my->edit_radios[i] =  gtk_radio_button_new_with_label_from_widget (
                    i == 0 ? NULL : GTK_RADIO_BUTTON(my->edit_radios[0]), noms[i]);
        g_object_set_data (G_OBJECT(my->edit_radios[i]), "numero", GINT_TO_POINTER(i));
        g_object_set_data (G_OBJECT(my->edit_radios[i]), "mode",  GINT_TO_POINTER(i + 1));
        g_signal_connect (my->edit_radios[i], "toggled", G_CALLBACK(on_radio_toggled), 
                          my);
        
        gtk_box_pack_start (GTK_BOX (vbox2), my->edit_radios[i], FALSE, FALSE, 0);
    }

    gtk_container_add (GTK_CONTAINER (my->frame), vbox2);
}

void layout_init (gpointer user_data){
    Mydata *my = get_mydata(user_data);
    
    my->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    my->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add (GTK_CONTAINER (my->window), my->vbox);
    my->scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (my->scroll), my->area);

    //Menu

    gtk_box_pack_start (GTK_BOX (my->vbox), my->menu_bar, FALSE, FALSE, 0);

    
    gtk_box_pack_start (GTK_BOX (my->hbox), my->frame, FALSE, FALSE, 2);

    gtk_box_pack_start (GTK_BOX (my->hbox), my->scroll, TRUE, TRUE, 0);

    gtk_box_pack_start (GTK_BOX (my->vbox), my->hbox, TRUE, TRUE, 0);

    //Area;

    //Status
    gtk_box_pack_start (GTK_BOX (my->vbox), my->status, FALSE, FALSE, 0);    
}

void menu_init (gpointer user_data){
    Mydata *my = get_mydata(user_data);
    
    GtkWidget *item_file, *item_tools, 
              *item_help, *sub_file, *item_load, *item_quit, *sub_tools,
              *item_rotate, *item_color, *item_scale, *sub_help, *item_about,
              *item_clip, *item_edit;
    
    my->menu_bar = gtk_menu_bar_new();    
    
    //Menu    
    item_file = gtk_menu_item_new_with_label ("File");
    item_tools = gtk_menu_item_new_with_label ("Tools");
    item_help = gtk_menu_item_new_with_label ("Help");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(my->menu_bar), item_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(my->menu_bar), item_tools);
    gtk_menu_shell_append(GTK_MENU_SHELL(my->menu_bar), item_help);
    
    //File
    sub_file = gtk_menu_new ();
    
    item_load = gtk_menu_item_new_with_label ("Load");
    item_quit = gtk_menu_item_new_with_label ("Quit");
    
    g_signal_connect (item_load, "activate",
                      G_CALLBACK(on_item_load_activate), my);
    g_signal_connect (item_quit, "activate",
                      G_CALLBACK(on_item_quit_activate), my);
                          
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_file), item_load);
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_file), item_quit);
    
    //Tools
    sub_tools = gtk_menu_new ();
    
    item_rotate = gtk_menu_item_new_with_label ("Rotate");
    item_color = gtk_menu_item_new_with_label ("Bg Color");
    item_scale = gtk_menu_item_new_with_label ("Scale");
    item_clip = gtk_check_menu_item_new_with_label ("Clip");
    item_edit = gtk_check_menu_item_new_with_label ("Editing");

    g_signal_connect (item_rotate, "activate",
                      G_CALLBACK(on_item_rotate_activate), my);
    g_signal_connect (item_color, "activate",
                      G_CALLBACK(on_item_color_activate), my);
    g_signal_connect (item_scale, "activate",
                      G_CALLBACK(on_item_scale_activate), my);                  
    g_signal_connect (item_clip, "activate",
                      G_CALLBACK(on_item_clip_activate), my);              
    g_signal_connect (item_edit, "activate",
                      G_CALLBACK(on_item_edit_activate), my);                      
                                              
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_tools), item_rotate);
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_tools), item_color);
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_tools), item_scale);
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_tools), item_clip);
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_tools), item_edit);
    
    //Help
    sub_help = gtk_menu_new ();
    
    item_about = gtk_menu_item_new_with_label ("About");
    
    g_signal_connect (item_about, "activate",
                      G_CALLBACK(on_item_about_activate), my);    
                      
    gtk_menu_shell_append(GTK_MENU_SHELL(sub_help), item_about);
    
    //Sub-menus    
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_file), sub_file);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_tools), sub_tools);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_help), sub_help);    
} 

void area_init (gpointer user_data){
    Mydata *my = get_mydata(user_data);    
                                 
    my->area = gtk_drawing_area_new ();    

    g_signal_connect (my->area, "draw", G_CALLBACK (on_area_draw), my);
    
    g_signal_connect (my->area, "key-press-event", G_CALLBACK (on_area_key_press), my);
    g_signal_connect (my->area, "key-release-event", G_CALLBACK (on_area_key_release), my);
    g_signal_connect (my->area, "button-press-event", G_CALLBACK (on_area_button_press), my);
    g_signal_connect (my->area, "button-release-event", G_CALLBACK (on_area_button_release), my);
    g_signal_connect (my->area, "motion-notify-event", G_CALLBACK (on_area_motion_notify), my);
    g_signal_connect (my->area, "enter-notify-event", G_CALLBACK (on_area_enter_notify), my);
    g_signal_connect (my->area, "leave-notify-event", G_CALLBACK (on_area_leave_notify), my);


    gtk_widget_add_events  (my->area,GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
                            GDK_FOCUS_CHANGE_MASK |
                            GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );
    
    gtk_widget_set_can_focus (my->area, TRUE);
}

void status_init (gpointer user_data){
    Mydata *my = get_mydata(user_data);        
    my->status = gtk_statusbar_new();
    set_status(my, "Welcome in TP2!");     
}

void win_scale_init (Mydata *my){    
    my->win_scale = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(my->win_scale), "Image scale :");    
    gtk_window_set_default_size (GTK_WINDOW(my->win_scale), 300, 100);
    
    GtkWidget *hbox, *label;
    
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    label = gtk_label_new ("Scale :");
    my->scale_horizon = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.02, 20.0, 0.02);
    gtk_range_set_value (GTK_RANGE(my->scale_horizon), 1.0);
    
    gtk_container_add (GTK_CONTAINER (my->win_scale), hbox);    
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 10);    
    gtk_box_pack_start (GTK_BOX (hbox), my->scale_horizon, TRUE, TRUE, 10);
    
    
    gtk_widget_show_all (my->win_scale);
    gtk_widget_hide (my->win_scale);

    
    g_signal_connect (my->scale_horizon, "value-changed",G_CALLBACK(on_scale_horizon_value_changed), my);
    g_signal_connect (my->win_scale, "delete-event",G_CALLBACK(gtk_widget_hide_on_delete), my);    
}

//Chargement de l'appli

void on_app_activate (GtkApplication* app, gpointer user_data){
    g_object_set (gtk_settings_get_default(),
                "gtk-shell-shows-menubar", FALSE, NULL);
    
    Mydata *my = get_mydata(user_data);
    
    window_init(app, my);    
    menu_init(my);    
    area_init(my);    
    status_init(my);
    editing_init(my);    
    layout_init(my);        
    win_scale_init(my);          
    gtk_widget_show_all (my->window);
    gtk_widget_hide (my->frame);    
}

int main (int argc, char *argv[]){
    GtkApplication *app;
    int status;
    Mydata my;
    init_mydata(&my);
    app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate",G_CALLBACK(on_app_activate), &my);
    status = g_application_run (G_APPLICATION(app), argc, argv);
    g_object_unref (app);
    return status;
}
