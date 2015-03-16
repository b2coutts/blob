
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
    vector<spoint> hull = giftwrap(included, excluded);

    for(auto& e : excluded) {
        if(point_inside(e, hull)) {
            cerr << "There's a excluded point inside!" << endl;
            cerr << "  ("<<e.x<<", "<<e.y<<")" << endl;
        }
    }

    return hull;
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
point_inside(const spoint &p, const std::vector<spoint> &points)
{
    bool inside = false;
    spoint e0 = points[points.size() - 1];
    bool y0 = (e0.y > p.y);
    for(int i = 0; i < points.size(); i++) {
        spoint e1 = points[i];
        bool y1 = (e1.y > p.y);
        if(y0 != y1) {
            // t == y1 is more efficient maybe, or we can just use t2 instead
            // as it it more obviously correct.  Should benchmark it.
            // bool t = ((e1.y - p.y) * (e0.x - e1.x) >= (e1.x - p.x) * (e0.y - e1.y));
            bool t2 = (p.x < ((e1.x-e0.x) * (p.y-e0.y) / (e1.y-e0.y)) + e0.x);
            if( t2 ) {
                inside = !inside;
                cerr << "Flipping inside to " << inside << endl;
                cerr << "  On iteration " << i << endl;
                cerr << "  Comparing points (" << e0.x << ", " << e0.y
                    << ") and (" << e1.x << ", " << e1.y << ")" << endl;
            }
        }
        e0 = e1;
        y0 = y1;
    }
    return inside;
}
