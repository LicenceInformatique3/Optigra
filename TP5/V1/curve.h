#ifndef CURVE_H
#define CURVE_H

#define CONTROL_MAX 100
#define CURVE_MAX 200

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

void init_curve_infos(Curve_infos *ci);

int add_curve (Curve_infos *ci);

int add_control (Curve_infos *ci, double x, double y);

int find_control (Curve_infos * ci, double x, double y);

int move_control (Curve_infos * ci, double dx, double dy);

int move_curve (Curve_infos * ci, double dx, double dy);

int remove_curve (Curve_infos * ci);

int remove_control (Curve_infos *ci);

#endif /* CURVE_H */
