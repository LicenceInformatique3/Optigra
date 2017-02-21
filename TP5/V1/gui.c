#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "curve.h"
#include "mydata.h"
#include "drawings.h"

void on_scale_horizon_value_changed (GtkWidget *widget, gpointer data){
    Mydata *my = get_mydata(data);
       if (gtk_range_get_value(GTK_RANGE(widget)) == my->scale_horizon_value) return;
    if (my->pixbuf2 != NULL) {
        my->scale_horizon_value = gtk_range_get_value(GTK_RANGE(widget));    
        update_area_with_transforms (my);    
    }
    
    else 
        set_status(my->status, "No image to scale.");
}

void on_radio_toggled (GtkWidget *widget, gpointer data) {
    Mydata *my = get_mydata(data);
    gint mode= GPOINTER_TO_INT(g_object_get_data (G_OBJECT(widget), "mode"));
    my->edit_mode = mode;
}

// Initialisations graphiques
void window_init (GtkApplication* app, gpointer user_data){
    Mydata *my = get_mydata(user_data);
    my->window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (my->window), my->title);
    gtk_window_set_default_size (GTK_WINDOW (my->window), my->win_width, my->win_height);
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
        g_signal_connect (my->edit_radios[i], "toggled", G_CALLBACK(on_radio_toggled), my);  
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

void status_init (gpointer user_data){
    Mydata *my = get_mydata(user_data);        
    my->status = gtk_statusbar_new();
    set_status(my->status, "Welcome in TP2!");     
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
