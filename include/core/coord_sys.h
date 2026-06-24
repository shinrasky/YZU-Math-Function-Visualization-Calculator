#ifndef COORD_SYS_H
#define COORD_SYS_H

#include "utils/common.h"

typedef enum {
    COORD_CARTESIAN_2D,
    COORD_POLAR,
    COORD_CARTESIAN_3D,
    COORD_CYLINDRICAL,
    COORD_SPHERICAL
} CoordSystem;

Point2D polar_to_cartesian(double r, double theta);
Point3D cylindrical_to_cartesian(double r, double theta, double z);
Point3D spherical_to_cartesian(double r, double theta, double phi);

#endif
