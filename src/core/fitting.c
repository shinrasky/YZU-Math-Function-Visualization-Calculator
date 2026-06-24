#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/fitting.h"

static int gaussian_elimination(double *a, double *b, double *x, int n) {
    int i;
    int j;
    int k;

    for (i = 0; i < n; i++) {
        int pivot = i;
        double max_abs = fabs(a[i * n + i]);
        for (j = i + 1; j < n; j++) {
            double v = fabs(a[j * n + i]);
            if (v > max_abs) {
                max_abs = v;
                pivot = j;
            }
        }
        if (max_abs < 1e-12) {
            return 0;
        }
        if (pivot != i) {
            for (k = i; k < n; k++) {
                double tmp = a[i * n + k];
                a[i * n + k] = a[pivot * n + k];
                a[pivot * n + k] = tmp;
            }
            {
                double tmpb = b[i];
                b[i] = b[pivot];
                b[pivot] = tmpb;
            }
        }

        for (j = i + 1; j < n; j++) {
            double factor = a[j * n + i] / a[i * n + i];
            for (k = i; k < n; k++) {
                a[j * n + k] -= factor * a[i * n + k];
            }
            b[j] -= factor * b[i];
        }
    }

    for (i = n - 1; i >= 0; i--) {
        double sum = b[i];
        for (j = i + 1; j < n; j++) {
            sum -= a[i * n + j] * x[j];
        }
        x[i] = sum / a[i * n + i];
    }

    return 1;
}

double *poly_fit(const Point2D *points, int count, int degree) {
    int n = degree + 1;
    int i;
    int j;
    int k;
    double *ata;
    double *aty;
    double *coeffs;

    if (!points || count <= degree || degree < 1 || degree > 5) {
        return NULL;
    }

    ata = (double *)calloc((size_t)(n * n), sizeof(double));
    aty = (double *)calloc((size_t)n, sizeof(double));
    coeffs = (double *)calloc((size_t)n, sizeof(double));
    if (!ata || !aty || !coeffs) {
        free(ata);
        free(aty);
        free(coeffs);
        return NULL;
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < count; k++) {
                sum += pow(points[k].x, i + j);
            }
            ata[i * n + j] = sum;
        }
        {
            double sumy = 0.0;
            for (k = 0; k < count; k++) {
                sumy += points[k].y * pow(points[k].x, i);
            }
            aty[i] = sumy;
        }
    }

    if (!gaussian_elimination(ata, aty, coeffs, n)) {
        free(ata);
        free(aty);
        free(coeffs);
        return NULL;
    }

    free(ata);
    free(aty);
    return coeffs;
}

void poly_to_expression(const double *coeffs, int degree, char *out_str, int max_len) {
    int i;
    int pos = 0;
    if (!coeffs || !out_str || max_len <= 0) {
        return;
    }

    out_str[0] = '\0';
    for (i = 0; i <= degree; i++) {
        char term[64];
        if (i == 0) {
            snprintf(term, sizeof(term), "%+.6g", coeffs[i]);
        } else if (i == 1) {
            snprintf(term, sizeof(term), "%+.6g*x", coeffs[i]);
        } else {
            snprintf(term, sizeof(term), "%+.6g*x^%d", coeffs[i], i);
        }
        if (pos + (int)strlen(term) < max_len - 1) {
            strcpy(out_str + pos, term);
            pos += (int)strlen(term);
        }
    }
}

double poly_eval(const double *coeffs, int degree, double x) {
    int i;
    double y = 0.0;
    for (i = degree; i >= 0; i--) {
        y = y * x + coeffs[i];
    }
    return y;
}
