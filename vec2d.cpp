#include "vec2d.h"
#include <math.h>

double inner(vec2d u, vec2d v){
    return u.x*v.x + u.y*v.y;
}

double norm(vec2d v){
    return sqrt(inner(v,v));
}

vec2d apply_matrix(double a, double b, double c, double d, vec2d v){
    vec2d ret = {a*v.x + b*v.y , c*v.x + d*v.y};
    return ret;
}

vec2d rotccw(vec2d v, double theta){
    return apply_matrix(cos(theta), -sin(theta),
                        sin(theta), cos(theta),
                        v);
}

vec2d scale(double scalar, vec2d v){
    vec2d ret = {scalar * v.x , scalar * v.y};
    return ret;
}

double dist(vec2d u, vec2d v){
    return norm(u-v);
}

vec2d stv(spoint p){
    return {p.x, p.y};
}

std::ostream& operator<<(std::ostream& out, const vec2d &v){
    out << "(" << v.x << "," << v.y << ")";
}
