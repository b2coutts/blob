#ifndef B2_H
#define B2_H

#include "types.h"
#include "vec2d.h"

#include <vector>
#include <list>

using namespace std;

// compute the "fixed" polygon
list<spoint> fixed_hull(vector<spoint> &inc, vector<spoint> &exc);

// returns the endpoints of a smooth line between the borders of the circles
// around a and b. a and b lie near a line l of the polygon; v is the normal
// vector to that line pointing out of the polygon
pair<vec2d,vec2d> smooth_line(spoint a, spoint b, vec2d v);

// refine a single line of a given polygon
void refine_line(list<spoint> &poly, list<spoint>::iterator ia,
                 list<spoint>::iterator ib, vector<spoint> &inc,
                 vector<spoint> &exc, double *incdists, double *excdists);

// given a fixed polygon, refine each of its lines
void refine_poly(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc);

#endif
