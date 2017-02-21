#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "curve.h"
#include "util.h"
#include "mydata.h"
#include "drawings.h"
#include "menus.h"
#include "font.h"

// Gestion DrawingArea : refresh, scale, rotation
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

void draw_control_polygons (cairo_t *cr, Curve_infos *ci) {
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

// Callbacks area
gboolean on_area_key_press (GtkWidget *area, GdkEvent *event, gpointer data){
    Mydata *my = get_mydata(data);
    GdkEventKey *evk = &event->key;
    printf ("%s: GDK_KEY_%s\n",    __func__, gdk_keyval_name(evk->keyval));
    switch (evk->keyval) {
        case GDK_KEY_q : gtk_widget_destroy(my->window); break;
        case GDK_KEY_a : set_edit_mode (my, EDIT_ADD_CURVE); break;
        case GDK_KEY_z : set_edit_mode (my, EDIT_MOVE_CURVE); break;
        case GDK_KEY_e : set_edit_mode (my, EDIT_REMOVE_CURVE); break;
        case GDK_KEY_r : set_edit_mode (my, EDIT_ADD_CONTROL); break;
        case GDK_KEY_t : set_edit_mode (my, EDIT_MOVE_CONTROL); break;
        case GDK_KEY_y : set_edit_mode (my, EDIT_REMOVE_CONTROL); break;
    }
    return TRUE;
}

gboolean on_area_key_release (GtkWidget *area, GdkEvent *event, gpointer data){
    GdkEventKey *evk = &event->key;
    printf ("%s: GDK_KEY_%s\n",    __func__, gdk_keyval_name(evk->keyval));
    return TRUE;
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
    PangoLayout *layout = pango_cairo_create_layout (cr);
    draw_control_polygons (cr, &my->curve_infos);
    draw_bezier_polygons_open(cr,&my->curve_infos);
    draw_control_labels(cr,layout,&my->curve_infos);
	g_object_unref (layout);
    return TRUE;
}

void draw_control_labels(cairo_t *cr, PangoLayout *layout, Curve_infos *ci){
	font_set_name(layout,"Sans, 8");
	cairo_set_source_rgb(cr,0.3,0.3,0.3);
	Curve * curve;
	for(int i = 0; i < ci->curve_list.curve_count;i++){
		curve = &ci->curve_list.curves[i];
		for(int j = 0; j < curve->control_count;j++){
			font_draw_text (cr, layout, FONT_TL,curve->controls[j].x - 3, curve->controls[j].y - 20, "%d",j);
		}
	}
	
}

void draw_bezier_polygons_open(cairo_t *cr,Curve_infos *ci){
	Curve * curve;
	Control bez_points[4];
	cairo_set_line_width (cr, 3);
	for(int i = 0;i < ci->curve_list.curve_count;++i){
		curve = &ci->curve_list.curves[i];
		for(int j = 0; j < curve->control_count - 3;++j){			
			compute_bezier_points(curve,j,bez_points);
            cairo_set_source_rgb (cr, 0, 1, 0);
            cairo_move_to (cr, curve->controls[0].x, curve->controls[0].y);
            cairo_line_to (cr, curve->controls[1].x, curve->controls[1].y);
            cairo_stroke (cr);
			cairo_move_to (cr, curve->controls[2].x, curve->controls[2].y);
            cairo_line_to (cr, curve->controls[3].x, curve->controls[3].y);
            cairo_stroke (cr);
		}
	}	
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
