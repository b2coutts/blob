
#include "types.h"
#include "blob.h"

#include <assert.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <list>
#include <math.h>
#include <vector>

using namespace std;

vector<spoint> giftwrap(vector<spoint> &included, vector<spoint> &excluded);
const double TAU = 6.28318530718;

const double EPSILON = 0.001;


vector<spoint> find_hull(vector<spoint> &included, vector<spoint> &excluded)
{

    return giftwrap(included, excluded);
}

vector<spoint> giftwrap(vector<spoint> &included, vector<spoint> &excluded) {
    // http://en.wikipedia.org/wiki/Gift_wrapping_algorithm
    list<spoint> inc(included.begin(), included.end());
    double leftmost = numeric_limits<double>::max();
    int leftmost_index = -1;

    vector<spoint> hull;
    hull.reserve(inc.size());
    // Find a point surely on the convex hull
    list<spoint>::const_iterator leftmost_iter = inc.begin();
    for(auto it = inc.begin(); it != inc.end(); it++) {
        if(it->x < leftmost) {
            leftmost = it->x;
            leftmost_iter = it;
        }
    }
    hull.push_back(*leftmost_iter);
    spoint base = *leftmost_iter;
    const spoint start = base;

    // inc.erase(leftmost_iter);

    double previous_angle = TAU/4;
    // Straight up is a ok way to reach the leftmost point if going
    // clockwise, as it only rules out points to it's left.
    // Buh?  It's going counterclockwise
    do {

        auto endit = inc.begin(); // The free end of the line
        if(endit == inc.end()) { break; }
        // while(*endit == base){ endit++; cerr << "WTF" << endl;}
        auto next = endit; // Our final choice of endpoint


        // Delta angles are wrt to lines
        // other angles are wrt the line and x-axis

        double best_delta_angle;
        double next_angle;
        double next_distance;
        { // Calc for first point
            double dx = endit->x - base.x;
            double dy = endit->y - base.y;
            double distance = dx*dx + dy*dy;
            double angle = atan2(dy, dx);
            double delta_angle = previous_angle - angle;
            while(delta_angle < 0) { delta_angle += TAU; }
            while(delta_angle > TAU) { delta_angle -= TAU; }

            best_delta_angle = delta_angle;
            next_angle = angle;
            next_distance = distance;
        }
        endit++;

        cerr << "Base: (" << base.x
            << ", " << base.y << ")" << endl;


        for(; endit != inc.end(); endit++) {
            while(*endit == base){ endit++; }
            spoint &end = *endit;
            //if(j == leftmost_index) { continue; }
            double dx = end.x - base.x;
            double dy = end.y - base.y;
            double distance = dx*dx + dy*dy;

            double angle = atan2(dy, dx);

            double delta_angle = previous_angle - angle;
            while(delta_angle < 0) { delta_angle += TAU; }
            while(delta_angle > TAU) { delta_angle -= TAU; }

            //assert(delta_angle < TAU/2);

            cerr << "  End: (" << end.x
                << ", " << end.y << ")" << endl;
            cerr << "    delta_angle: " << delta_angle << endl;
            double delta_delta_angle = abs(best_delta_angle - delta_angle);
            if( delta_delta_angle < EPSILON ) {
                // If on the same line, pick the closer one
                if(next_distance > distance) {
                    cerr << "    New closer point" << endl;
                    next = endit;
                    best_delta_angle = delta_angle;
                    next_angle = angle;
                    next_distance = distance;
                }
            } else if(best_delta_angle > delta_angle) {
                cerr << "    New best angle" << endl;
                next = endit;
                best_delta_angle = delta_angle;
                next_angle = angle;
                next_distance = distance;
            }
        }

        hull.push_back(*next);
        base = *next;
        previous_angle = next_angle;
        inc.erase(next);
    } while (! (base == start));

    // oops, it repeats
    (void) hull.pop_back();


    return hull;
}

bool
point_inside(const std::vector<spoint> &points)
{
    return false;
}
