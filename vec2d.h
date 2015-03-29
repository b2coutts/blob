// TODO: should this just all be included in spoint?
#ifndef VEC2D_H
#define VEC2D_H

#include "types.h"

struct vec2d {
    double x,y;
};

inline bool operator==(const vec2d &u, const vec2d &v) {
    return u.x == v.x && u.y == v.y;
}

inline vec2d operator+(const vec2d &u, const vec2d &v) {
    vec2d ret = {u.x + v.x, u.y + v.y};
    return ret;
}

inline vec2d operator-(const vec2d &v) {
    vec2d ret = {-v.x, -v.y};
    return ret;
}

inline vec2d operator-(const vec2d &u, const vec2d &v) {
    vec2d ret = {u.x - v.x, u.y - v.y};
    return ret;
}

// inner product
double inner(vec2d u, vec2d v);

// norm
double norm(vec2d v);

// apply the matrix [a b; c d] to the vector v
vec2d apply_matrix(double a, double b, double c, double d, vec2d v);

// rotate the given vector counter-clockwise by theta (given in radians)
vec2d rotccw(vec2d v, double theta);

// TODO: can this somehow be implemented by overloading the multiplication
// operator?
// scales a vector by a scalar
vec2d scale(double scalar, vec2d v);

// distance between two vectors
double dist(vec2d u, vec2d v);

// converts a spoint to a vec2d
vec2d stv(spoint p);

#endif
