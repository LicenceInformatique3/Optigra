#ifndef DRAWINGS_H
#define DRAWINGS_H

void apply_image_transforms (Mydata *data);
void update_area_with_transforms (Mydata *data);
void draw_curves (cairo_t *cr, Curve_infos *ci);
void area_init (gpointer user_data);

#endif // Fin DRAWINGS_H 
