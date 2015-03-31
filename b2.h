#ifndef B2_H
#define B2_H

#include "types.h"
#include "vec2d.h"

#include <vector>
#include <list>

using namespace std;

// helper function for writing a polygon to stdout
void print_poly(list<spoint> poly);

// compute the "fixed" polygon
list<spoint> fixed_hull(vector<spoint> &inc, vector<spoint> &exc);

// returns the endpoints of a smooth line between the borders of the circles
// around a and b. a and b lie near a line l of the polygon; v is the normal
// vector to that line pointing out of the polygon
pair<double,double> smooth_line_angle(spoint sa, spoint sb, double ra, double rb);

// refine a single line of a given polygon
bool refine_line(list<spoint> &poly, list<spoint>::iterator ia,
                 list<spoint>::iterator ib, vector<spoint> &inc,
                 vector<spoint> &exc, double *incdists, double *excdists);

// given a fixed polygon, refine each of its lines
void refine_poly(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc);

// given a refined polygon, remove vertices that would cause crossing lines
// (i.e., where the interpolant would loop around that point and
// self-intersect)
void rm_crossing(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc);

// computes the radii of the vertices of a polygon, for the interpolant
vector<double> get_radii(const vector<spoint> &points,
                          vector<spoint> &inpoints, vector<spoint> &expoints);

#endif
