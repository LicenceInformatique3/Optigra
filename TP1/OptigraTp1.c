#include <gtk/gtk.h>
 
#define MYDATA_MAGIC 0x46EA7E05
 
typedef struct {
    GtkWidget *window;
    GtkWidget *image;
    char *title;
    int win_width;
    int win_height;
    unsigned int magic;
    char * current_folder;
} Mydata;
 
void init_mydata(Mydata *my){
    my->title = "TP1 en GTK";
    my->win_width = 400;
    my->win_height = 300;
    my->magic = MYDATA_MAGIC;
    my->current_folder = NULL;
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
 
void on_button1_clicked (GtkWidget *widget, gpointer data)
{
    Mydata *my = get_mydata(data);
    
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;gint res;
    dialog = gtk_file_chooser_dialog_new ("Load Image",my->window,action,"Cancel",GTK_RESPONSE_CANCEL,"Open",GTK_RESPONSE_ACCEPT,NULL);
    if(my->current_folder != NULL){
         gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog),my->current_folder);
    }
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        gtk_image_set_from_file(GTK_IMAGE(my->image),filename);
        free(my->current_folder);
        my->current_folder = gtk_file_chooser_get_current_folder(chooser);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
    printf ("Button clicked in window ’%s’\n", my->title);
}
 
void on_b_quit_clicked (GtkWidget *widget, gpointer data)
{
    Mydata *my = get_mydata(data);
    printf ("Closing window and app\n");
    gtk_widget_destroy(my->window);
}
 
void on_app_activate (GtkApplication* app, gpointer user_data)
{
    Mydata *my = get_mydata(user_data);
    GtkWidget *button1,*b_quit,*vbox1,*hbox1,*b_bottom;
    
    my->window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (my->window), my->title);
    gtk_window_set_default_size (GTK_WINDOW (my->window),my->win_width, my->win_height);
    
    
    vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (my->window), vbox1);
    
    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox1),hbox1, FALSE, FALSE, 0);
    
    my->image = gtk_image_new();
    
    //bouton1
    button1 = gtk_button_new_with_label ("Load Image");
    gtk_box_pack_start (GTK_BOX (hbox1), button1, TRUE, TRUE, 0);
    g_signal_connect (button1, "clicked",G_CALLBACK(on_button1_clicked), my);
    
    //bouton quit
    b_quit = gtk_button_new_with_label ("Quit");
    gtk_box_pack_start (GTK_BOX (hbox1), b_quit,FALSE,FALSE,0);
    g_signal_connect (b_quit, "clicked",G_CALLBACK(on_b_quit_clicked), my);
    
    //image Danseur très très chaud
    gtk_box_pack_start (GTK_BOX (vbox1), my->image,TRUE,TRUE,0);
    
    //b_bottom
    b_bottom = gtk_button_new_with_label ("Bottom");
    gtk_box_pack_start (GTK_BOX (vbox1), b_bottom,FALSE,FALSE,0);
    
    
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
