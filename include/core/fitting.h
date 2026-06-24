#ifndef FITTING_H
#define FITTING_H

#include "utils/common.h"

double *poly_fit(const Point2D *points, int count, int degree);
void poly_to_expression(const double *coeffs, int degree, char *out_str, int max_len);
double poly_eval(const double *coeffs, int degree, double x);

#endif
