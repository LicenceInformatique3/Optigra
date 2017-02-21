#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "curve.h"
#include "mydata.h"
#include "drawings.h"
#include "menus.h"

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
        set_status(my->status, "Loading image ...");
        g_clear_object(&my->pixbuf);
        my->pixbuf = gdk_pixbuf_new_from_file(filename, NULL);       
        if (my->pixbuf == NULL) { 
            set_status(my->status, "Loading failed : not an image.");
        }
        else {
            set_status(my->status, "Loading success : image %dx%d.",gdk_pixbuf_get_width(my->pixbuf),gdk_pixbuf_get_height(my->pixbuf));
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

void on_item_about_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    char *auteurs[] = {"Ahmed Belamri <ahmed.belamri.1@etu.univ-amu.fr>", NULL};
    gtk_show_about_dialog (NULL, "program-name", my->title, "version", 
                           "2.4", "website", "http://j.mp/optigra", 
                           "authors", auteurs, "logo-icon-name", 
                           "face-wink",NULL);    
    printf ("About\n");
}

void on_item_quit_activate (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
    gtk_widget_destroy(my->window);
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
        set_status (my->status, "Selected bg color : %s", c);
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
        
        set_status(my->status, "Image rotated.");
    }
    else 
        set_status(my->status, "No image to rotate.");
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
        set_status(my->status, "Clipping is on");
    else
        set_status(my->status, "Clipping is off");
        
    refresh_area (my->area);
}

void on_item_edit_activate (GtkCheckMenuItem *widget, gpointer data){
    Mydata *my = get_mydata(data);    
    
    my->show_edit = gtk_check_menu_item_get_active (widget);
    if (my->show_edit == TRUE) {
        set_status(my->status, "Editing is on");
        gtk_widget_show (my->frame);
    }
    else {
        set_status(my->status, "Editing is off");
        gtk_widget_hide (my->frame);
    }
        
    refresh_area (my->area);
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
