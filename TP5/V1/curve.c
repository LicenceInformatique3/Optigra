#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curve.h"

void init_curve_infos(Curve_infos *ci){
	ci->curve_list.curve_count = 0;
	ci->current_control = -1;
	ci->current_curve = -1;
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
