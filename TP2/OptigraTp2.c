#include <gtk/gtk.h>
#include <stdlib.h>

#define MYDATA_MAGIC 0x46EA7E05

typedef struct {
    GtkWidget *window;
    GtkWidget *image;
    GtkWidget *status;
    GtkWidget *vbox1;
    GtkWidget *win_scale;
    double scale_horrizon_value;
    char *title;
    int win_width;
    int win_height;
    unsigned int magic;
    char * current_folder;
    GdkPixbuf * pixbuf;
} Mydata;

void init_mydata(Mydata *my){
    my->title = "TP1 en GTK";
    my->win_width = 400;
    my->win_height = 300;
    my->magic = MYDATA_MAGIC;
    my->current_folder = NULL;
    my->pixbuf = NULL;
    my->scale_horrizon_value = 1.0;
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
        if(my->pixbuf == NULL){
            set_status(my,"Loading Fail !: not an image...");
            gtk_image_set_from_icon_name(GTK_IMAGE(my->image),"image-missing",GTK_ICON_SIZE_DIALOG);
        }else{
            char msg[80];
            sprintf(msg,"Loading success : image %dx%d",gdk_pixbuf_get_width(my->pixbuf),gdk_pixbuf_get_height(my->pixbuf));
            set_status(my,msg);
            gtk_image_set_from_pixbuf(GTK_IMAGE(my->image),my->pixbuf);
        }
        //gtk_image_set_from_file(GTK_IMAGE(my->image),filename);
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
        GdkPixbuf *tmp = gdk_pixbuf_rotate_simple(my->pixbuf,90);
        g_object_unref(my->pixbuf);
        my->pixbuf = tmp;
        gtk_image_set_from_pixbuf(GTK_IMAGE(my->image),my->pixbuf);
        set_status(my,"Rotate dans ta face !");
        printf ("Rotate\n");
    }else{
        printf ("Pas Rotate\n");
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
        char str[1000];
        sprintf(str,"Selected bg color : %s",color);
        set_status(my,str);
        gtk_widget_override_background_color(my->image,GTK_STATE_FLAG_NORMAL,&bg_color);
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

void on_scale_horrizon_value_changed(GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    if(my->pixbuf != NULL){
        int width = gdk_pixbuf_get_width(my->pixbuf);
        int height = gdk_pixbuf_get_height(my->pixbuf);
        my->scale_horrizon_value = gtk_range_get_value(GTK_RANGE(widget));
        gtk_range_set_value(GTK_RANGE(widget),my->scale_horrizon_value);
        char str[100];
        sprintf(str,"scale = %f",my->scale_horrizon_value);
        set_status(my,str);
        GdkPixbuf *tmp_pixbuf = gdk_pixbuf_scale_simple(my->pixbuf,(width*my->scale_horrizon_value),(height*my->scale_horrizon_value),GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf(GTK_IMAGE(my->image),tmp_pixbuf);
        g_clear_object(&tmp_pixbuf);
    }
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

    //sub_help
    sub_help = gtk_menu_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(sub_help), item_about);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(item_help), sub_help);
    
    
    g_object_set (gtk_settings_get_default(),"gtk-shell-shows-menubar", FALSE, NULL);
}

void image_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    GtkWidget *scroll;
    //scroll
    scroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_box_pack_start(GTK_BOX (my->vbox1),scroll,TRUE,TRUE,0); 
    my->image = gtk_image_new();
    //image Danseur très très chaud hot +18, warning !
    gtk_container_add (GTK_CONTAINER (scroll), my->image);
}

void status_init(gpointer user_data){
    Mydata *my = get_mydata(user_data);
    //status
    my->status = gtk_statusbar_new();
    set_status(my,"Wellcome in TP1");
    gtk_box_pack_start (GTK_BOX (my->vbox1), my->status,FALSE,FALSE,0);
}

void win_scale_init(Mydata *my){
    my->win_scale = gtk_window_new(GTK_WINDOW_TOPLEVEL);    
    GtkWidget *box_horrizon,*label_horrizon,*scale_horrizon;
    gtk_window_set_title(GTK_WINDOW(my->win_scale),"Image scale");
    g_signal_connect (my->win_scale, "delete-event",G_CALLBACK(gtk_widget_hide_on_delete), my);
    gtk_window_set_default_size (GTK_WINDOW (my->win_scale),300, 100);
    
    box_horrizon = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (my->win_scale), box_horrizon);
    
    //label
    label_horrizon = gtk_label_new("Scale:");
    gtk_box_pack_start (GTK_BOX (box_horrizon), label_horrizon, FALSE, FALSE, 10);
    
    //scale
    scale_horrizon = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0.02,20.0,0.02);
    gtk_range_set_value(GTK_RANGE(scale_horrizon),my->scale_horrizon_value);
    g_signal_connect (scale_horrizon, "value-changed",G_CALLBACK(on_scale_horrizon_value_changed), my);
    gtk_box_pack_start (GTK_BOX (box_horrizon), scale_horrizon, TRUE, TRUE, 10);

    gtk_widget_show_all (my->win_scale);
    gtk_widget_hide(my->win_scale);
}

void on_app_activate (GtkApplication* app, gpointer user_data)
{
    Mydata *my = get_mydata(user_data);
    windows_init(app,my);
    layout_init(my);
    menu_init(my);
    image_init(my);
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

