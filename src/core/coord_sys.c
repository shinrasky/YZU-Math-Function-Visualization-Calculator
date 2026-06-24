#include <math.h>

#include "core/coord_sys.h"

Point2D polar_to_cartesian(double r, double theta) {
    Point2D p;
    p.x = r * cos(theta);
    p.y = r * sin(theta);
    return p;
}

Point3D cylindrical_to_cartesian(double r, double theta, double z) {
    Point3D p;
    p.x = r * cos(theta);
    p.y = r * sin(theta);
    p.z = z;
    return p;
}

Point3D spherical_to_cartesian(double r, double theta, double phi) {
    Point3D p;
    p.x = r * sin(phi) * cos(theta);
    p.y = r * sin(phi) * sin(theta);
    p.z = r * cos(phi);
    return p;
}
