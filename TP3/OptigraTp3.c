#include <gtk/gtk.h>
#include <stdlib.h>

#define MYDATA_MAGIC 0x46EA7E05

typedef struct {
    GtkWidget *window;
    //GtkWidget *image;
    GtkWidget *status;
    GtkWidget *vbox1;
    GtkWidget *win_scale;
    GtkWidget *scale_horizon;
    GtkWidget *area;
    double scale_horizon_value;
    double rotate_angle;
    char *title;
    int win_width;
    int win_height;
    unsigned int magic;
    char * current_folder;
    GdkPixbuf * pixbuf, *pixbuf2;
    double click_x;
    double click_y;
    int click_n;
    int clip_image;
} Mydata;

void init_mydata(Mydata *my){
    my->title = "TP1 en GTK";
    my->win_width = 400;
    my->win_height = 300;
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
    my->clip_image = FALSE;
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

void refresh_area (GtkWidget *area)
{
    GdkWindow *win = gtk_widget_get_window (area);
    if (win == NULL) return;
    gdk_window_invalidate_rect (win,NULL,FALSE);
}

void apply_image_transforms (Mydata *data) {    
    Mydata *my = get_mydata(data);    
    g_clear_object(&my->pixbuf2);
    if (my->pixbuf == NULL) return;
    GdkPixbuf *tmp = gdk_pixbuf_rotate_simple (my->pixbuf, my->rotate_angle);                
    int width = gdk_pixbuf_get_width (tmp);
    int height = gdk_pixbuf_get_height (tmp);    
    
    my->pixbuf2 = gdk_pixbuf_scale_simple (tmp, width * my->scale_horizon_value, height * my->scale_horizon_value, GDK_INTERP_BILINEAR);
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

void set_status(Mydata *my, const char *msg){
    gtk_statusbar_pop(GTK_STATUSBAR(my->status),0);
    gtk_statusbar_push(GTK_STATUSBAR(my->status),0,msg);
}

void on_item_load_activate(GtkWidget *widget, gpointer data){
    
    Mydata *my = get_mydata(data);
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;gint res;
    dialog = gtk_file_chooser_dialog_new ("Load Image",GTK_WINDOW(my->window),action,"Cancel",GTK_RESPONSE_CANCEL,"Open",GTK_RESPONSE_ACCEPT,NULL);
    if(my->current_folder != NULL){
         gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog),my->current_folder);
    }
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        set_status(my,"Loading Image...");
        g_clear_object(&my->pixbuf);
        my->pixbuf = gdk_pixbuf_new_from_file(filename,NULL);
        my->scale_horizon_value = 1.0;
        gtk_range_set_value(GTK_RANGE(my->scale_horizon),my->scale_horizon_value);
        
        if(my->pixbuf == NULL){
            set_status(my,"Loading Fail !: not an image...");
            //gtk_image_set_from_icon_name(GTK_IMAGE(my->image),"image-missing",GTK_ICON_SIZE_DIALOG);
        }else{
            char msg[80];
            int width = gdk_pixbuf_get_width(my->pixbuf);
            int height = gdk_pixbuf_get_height(my->pixbuf);
            sprintf(msg,"Loading success : image %dx%d",width,height);
            set_status(my,msg);
            my->rotate_angle = 0.0;
            apply_image_transforms (my);
            //gtk_image_set_from_pixbuf(GTK_IMAGE(my->image),my->pixbuf);
        }
        free(my->current_folder);
        my->current_folder = gtk_file_chooser_get_current_folder(chooser);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
    printf ("Button clicked in window ’%s’\n", my->title);
}

void on_item_quit_activate(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    printf ("Closing window and app\n");
    gtk_widget_destroy(my->window);
}

void on_item_rotate_activate(GtkWidget *widgetm, gpointer data){
    Mydata *my = get_mydata(data);
    if(my->pixbuf != NULL){
        //GdkPixbuf *tmp = gdk_pixbuf_rotate_simple(my->pixbuf,90);
        //g_object_unref(my->pixbuf);
        //my->pixbuf = tmp;
        //int width = gdk_pixbuf_get_width(my->pixbuf);
        //int height = gdk_pixbuf_get_height(my->pixbuf);
        //GdkPixbuf *tmp_pixbuf = gdk_pixbuf_scale_simple(my->pixbuf,(width*my->scale_horizon_value),(height*my->scale_horizon_value),GDK_INTERP_BILINEAR);
        //gtk_image_set_from_pixbuf(GTK_IMAGE(my->image),tmp_pixbuf);
        
        //g_clear_object(&tmp_pixbuf);
        
        my->rotate_angle = my->rotate_angle + 270;
        if (my->rotate_angle >= 360) 
            my->rotate_angle = my->rotate_angle - 360;
        update_area_with_transforms (my);
        set_status(my, "Image rotated.");
    }
    else{ 
        set_status(my, "No image to rotate.");
    }
}

void on_item_bgcolor_activate(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    GtkWidget *chooser_color;
    gint res = NULL;
    chooser_color = gtk_color_chooser_dialog_new("Background color",NULL);
    res = gtk_dialog_run(GTK_DIALOG(chooser_color));
    if(res == GTK_RESPONSE_OK){
        GdkRGBA bg_color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(chooser_color),&bg_color);
        gchar * color;
        color = gdk_rgba_to_string(&bg_color);
        char str[80];
        sprintf(str,"Selected bg color : %s",color);
        set_status(my,str);
        gtk_widget_override_background_color(my->area,GTK_STATE_FLAG_NORMAL,&bg_color);
        g_free(color);
    }
    gtk_widget_destroy(chooser_color);
}

void on_item_about_activate(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    char *auteurs[] = {"Florian Duarte <florian.duarte@etu.univ-amu.fr>","Gaetan Perrot <gaetan.perrot@etu.univ-amu.fr>",NULL};
    gtk_show_about_dialog(NULL,"program_name",my->title,"version","2.4","website","http://j.mp/optigra","authors",auteurs,
    "logo-icon-name","face-devilish",NULL);
}

void on_item_scale_activate(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    gtk_window_present(GTK_WINDOW(my->win_scale));
}

void on_scale_horizon_value_changed(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    
    if(my->pixbuf2 != NULL){
        my->scale_horizon_value = gtk_range_get_value(GTK_RANGE(widget));
        gtk_range_set_value(GTK_RANGE(widget),my->scale_horizon_value);
        update_area_with_transforms(my);
    }else{
        set_status(my,"No image to scale.");
    }
}

gboolean on_area_draw (GtkWidget *area, cairo_t *cr, gpointer data)
{
    Mydata *my = get_mydata(data);
    if(my->area != NULL){
        
        int width  = gtk_widget_get_allocated_width  (area);
        int    height = gtk_widget_get_allocated_height (area);
        char str[80];
        sprintf(str,"New size: %d * %d",width,height);
        set_status(my,str);
    }
    
    if(my->pixbuf2 != NULL){
        int pix_width = gdk_pixbuf_get_width(my->pixbuf2);
        int pix_height = gdk_pixbuf_get_height(my->pixbuf2);
        gdk_cairo_set_source_pixbuf(cr,my->pixbuf2,0,0);
        if (my->clip_image == FALSE)
        {
            cairo_rectangle (cr, 0.0, 0.0, pix_width, pix_height);
            cairo_fill (cr);
        }
    }
    
    if (my->click_n == 1)
    {
        cairo_set_line_width (cr, 2);
        cairo_set_source_rgb (cr, 0, 0, 1.0);
        cairo_arc (cr, my->click_x, my->click_y, 100.0, 0, 2 * G_PI);
        cairo_stroke (cr);
        if ((my->pixbuf2 != NULL) && (my->clip_image == TRUE))
        {
            gdk_cairo_set_source_pixbuf (cr, my->pixbuf2, 0, 0);
            cairo_arc (cr, my->click_x, my->click_y, 100.0, 0, 2 * G_PI);
            cairo_fill (cr);
        }
    }
    return TRUE;
}

void on_item_clip_activate (GtkCheckMenuItem *widget, gpointer data)
{
    Mydata *my = get_mydata(data);
    
    my->clip_image = gtk_check_menu_item_get_active (widget);
    if (my->clip_image == TRUE)
        set_status(my, "Clipping is on");
    else
        set_status(my, "Clipping is off");
        
    refresh_area (my->area);
}

gboolean on_area_key_press (GtkWidget *area, GdkEvent *event, gpointer data)
{
    Mydata *my = get_mydata(data);
    GdkEventKey *evk = &event->key;
    printf ("%s: GDK_KEY_%s\n",__func__, gdk_keyval_name(evk->keyval));
    switch (evk->keyval) {
    case GDK_KEY_q : gtk_widget_destroy(my->window); break;
    }
    return TRUE;  // evenement traite
}

gboolean on_area_enter_notify (GtkWidget *area, GdkEvent *event, gpointer data)
{
    Mydata *my = get_mydata(data);
    GdkEventCrossing *evc = &event->crossing;
    printf ("%s: %.1f %.1f\n", __func__, evc->x, evc->y);
    gtk_widget_grab_focus (my->area);
    return TRUE;
}

gboolean on_area_key_release (GtkWidget *area, GdkEvent *event, gpointer data){
    return TRUE;
}

gboolean on_area_button_press (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    GdkEventButton *evb = &event->button;
    my->click_n = evb->button;
    my->click_x = evb->x;
    my->click_y = evb->y;
    printf ("%s: %d %.1f %.1f\n", __func__, evb->button, evb->x, evb->y);
    refresh_area(area);
    return TRUE;
}

gboolean on_area_button_release (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    my->click_n = 0;    
    refresh_area(area);
    return TRUE;
}

gboolean on_area_motion_notify (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    GdkEventMotion *evm = &event->motion;
    
    if (my->click_n == 1)
    {
        my->click_x = evm->x;
        my->click_y = evm->y;
        printf ("%s: %.1f %.1f\n", __func__, evm->x, evm->y);
        
        refresh_area(area);
    }
    
    return TRUE;
}

gboolean on_area_leave_notify (GtkWidget *area, GdkEvent *event, gpointer data){
    return TRUE;
}


void windows_init(GtkApplication* app, gpointer user_data){
    Mydata *my = get_mydata(user_data);
    my->window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (my->window), my->title);
    gtk_window_set_default_size (GTK_WINDOW (my->window),my->win_width, my->win_height);
}

void layout_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    //vbox1
    my->vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (my->window), my->vbox1);
}

void menu_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    GtkWidget *menu_bar,*item_file,*item_tools,*item_help,*sub_file,*item_load,*item_quit,*sub_tools,*sub_help,*item_rotate,*item_bgcolor,*item_about,*item_scale;
    //menu_bar
    menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start (GTK_BOX (my->vbox1),menu_bar, FALSE, FALSE, 0);
 
    //items
    item_file = gtk_menu_item_new_with_label("File");
    item_tools = gtk_menu_item_new_with_label("Tools");
    item_help = gtk_menu_item_new_with_label("Help");
    
    gtk_menu_shell_append (GTK_MENU_SHELL(menu_bar),item_file);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu_bar),item_tools);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu_bar),item_help);
    
    //item subfile
    item_load = gtk_menu_item_new_with_label("Load");
    g_signal_connect (item_load, "activate",G_CALLBACK(on_item_load_activate), my);
    
    item_quit = gtk_menu_item_new_with_label("Quit");
    g_signal_connect (item_quit, "activate",G_CALLBACK(on_item_quit_activate), my);
    
    //item subtools
    item_rotate = gtk_menu_item_new_with_label("Rotate");
    g_signal_connect (item_rotate, "activate",G_CALLBACK(on_item_rotate_activate), my);
    
    item_bgcolor = gtk_menu_item_new_with_label("Bg Color");
    g_signal_connect (item_bgcolor, "activate",G_CALLBACK(on_item_bgcolor_activate), my);
    
    item_scale = gtk_menu_item_new_with_label("Scale");
    g_signal_connect (item_scale, "activate",G_CALLBACK(on_item_scale_activate), my);
    
    //item subhelp
    item_about = gtk_menu_item_new_with_label("About");
    g_signal_connect (item_about, "activate",G_CALLBACK(on_item_about_activate), my);
    
    //sub_File
    sub_file = gtk_menu_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_file), item_load);
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_file), item_quit);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_file), sub_file);
    
    //sub_tools
    sub_tools = gtk_menu_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_tools), item_rotate);
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_tools), item_bgcolor);
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_tools), item_scale);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_tools), sub_tools);

    //item_clip
    GtkWidget *item_clip = gtk_check_menu_item_new_with_label ("Clip");
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_tools), item_clip);
    g_signal_connect (item_clip, "activate", G_CALLBACK(on_item_clip_activate), my);
    
    //sub_help
    sub_help = gtk_menu_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_help), item_about);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_help), sub_help);
    
    
    g_object_set (gtk_settings_get_default(),"gtk-shell-shows-menubar", FALSE, NULL);
}

void area_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    GtkWidget *scroll;
    scroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_box_pack_start(GTK_BOX (my->vbox1),scroll,TRUE,TRUE,0); 
    //my->image = gtk_image_new();
    my->area = gtk_drawing_area_new ();
    //image Danseur très très chaud hot +18, warning !
    //gtk_container_add (GTK_CONTAINER (scroll), my->image);
    gtk_container_add (GTK_CONTAINER (scroll), my->area);
    //gtk_widget_set_size_request (my->area, 600, 400);
    g_signal_connect (my->area, "draw",G_CALLBACK (on_area_draw), my);
    g_signal_connect (my->area, "key-press-event",G_CALLBACK (on_area_key_press), my);
    g_signal_connect (my->area, "key-release-event",G_CALLBACK (on_area_key_release), my);
    g_signal_connect (my->area, "button-press-event",G_CALLBACK (on_area_button_press), my);
    g_signal_connect (my->area, "button-release-event",G_CALLBACK (on_area_button_release), my);
    g_signal_connect (my->area, "motion-notify-event",G_CALLBACK (on_area_motion_notify), my);
    g_signal_connect (my->area, "enter-notify-event",G_CALLBACK (on_area_enter_notify), my);
    g_signal_connect (my->area, "leave-notify-event",G_CALLBACK (on_area_leave_notify), my);
    gtk_widget_set_can_focus (my->area, TRUE);
    gtk_widget_add_events (my->area,GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |GDK_FOCUS_CHANGE_MASK |
    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |GDK_POINTER_MOTION_MASK |GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
}

void status_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    //status
    my->status = gtk_statusbar_new();
    set_status(my,"Wellcome in TP3");
    gtk_box_pack_start (GTK_BOX (my->vbox1), my->status,FALSE,FALSE,0);
}

void win_scale_init(Mydata *my){
    my->win_scale = gtk_window_new(GTK_WINDOW_TOPLEVEL);    
    GtkWidget *box_horizon,*label_horizon;
    gtk_window_set_title(GTK_WINDOW(my->win_scale),"Image scale");
    g_signal_connect (my->win_scale, "delete-event",G_CALLBACK(gtk_widget_hide_on_delete), my);
    gtk_window_set_default_size (GTK_WINDOW (my->win_scale),300, 100);
    
    box_horizon = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (my->win_scale), box_horizon);
    
    //label
    label_horizon = gtk_label_new("Scale:");
    gtk_box_pack_start (GTK_BOX (box_horizon), label_horizon, FALSE, FALSE, 10);
    
    //scale
    my->scale_horizon = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0.02,20.0,0.02);
    gtk_range_set_value(GTK_RANGE(my->scale_horizon),1.0);
    g_signal_connect (my->scale_horizon, "value-changed",G_CALLBACK(on_scale_horizon_value_changed), my);
    gtk_box_pack_start (GTK_BOX (box_horizon), my->scale_horizon, TRUE, TRUE, 10);

    gtk_widget_show_all (my->win_scale);
    gtk_widget_hide(my->win_scale);
}

void on_app_activate (GtkApplication* app, gpointer user_data)
{
    Mydata *my = get_mydata(user_data);
    windows_init(app,my);
    layout_init(my);
    menu_init(my);
    area_init(my);
    status_init(my);
    win_scale_init(my);
    gtk_widget_show_all (my->window);
}

int main (int argc, char *argv[])
{
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
