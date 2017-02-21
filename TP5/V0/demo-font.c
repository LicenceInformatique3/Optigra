/* demo-font.c
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 18/02/2016
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

#include <gtk/gtk.h>
#include "font.h"


typedef struct {
  GtkWidget *window, *area;
  char *title, *markup_text, *font_name;
} Mydata;


void mydata_init (Mydata *my)
{
  my->title       = "Démo du module font.c";
  my->markup_text = "<b>Gras</b> <i>italique</i> <u>souligné</u>";
  my->font_name   = "sans 12";
}


gboolean on_area_draw (GtkWidget *area, cairo_t *cr, gpointer data)
{
  Mydata *my = data;

  int w = gtk_widget_get_allocated_width (area),
      h = gtk_widget_get_allocated_height (area);

  PangoLayout *layout = pango_cairo_create_layout (cr);

  font_set_name (layout, my->font_name);
  font_draw_text (cr, layout, FONT_TL,   2,   2, "Top\nLeft");
  font_draw_text (cr, layout, FONT_TC, w/2,   2, "Top\nCenter");
  font_draw_text (cr, layout, FONT_TR, w-2,   2, "Top\nRight");
  font_draw_text (cr, layout, FONT_ML,   2, h/2, "Middle\nLeft");
  font_draw_text (cr, layout, FONT_MR, w-2, h/2, "Middle\nRight");
  font_draw_text (cr, layout, FONT_BL,   2, h-2, "Bottom\nLeft");
  font_draw_text (cr, layout, FONT_BC, w/2, h-2, "Bottom\nCenter");
  font_draw_text (cr, layout, FONT_BR, w-2, h-2, "Bottom\nRight");

  cairo_set_source_rgb (cr, 210/255., 105/255., 30/255.);
  for (int i = 0; i < 5; i++) {
    int size = 10+i*3;
    font_set_size (layout, size);
    font_draw_markup (cr, layout, FONT_MC, w/2, h/2 + 25*(i-2), 
        "%s %d", my->markup_text, size); 
  }

  g_object_unref (layout);
  return TRUE;  // Event processed, don't propagate further.
}


void on_app_activate (GtkApplication* app, gpointer user_data)
{
  Mydata *my = user_data;

  my->window = gtk_application_window_new (app);

  gtk_window_set_title (GTK_WINDOW (my->window), my->title);
  gtk_window_set_default_size (GTK_WINDOW (my->window), 400, 300);

  my->area = gtk_drawing_area_new ();
  gtk_container_add (GTK_CONTAINER (my->window), my->area);
  g_signal_connect (my->area, "draw", G_CALLBACK(on_area_draw), my);

  gtk_widget_show_all (my->window);
}


int main (int argc, char *argv[])
{
  Mydata my;
  mydata_init (&my);

  GtkApplication *app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK(on_app_activate), &my);

  int status = g_application_run (G_APPLICATION(app), argc, argv);
  g_object_unref (app);
  return status;
}

