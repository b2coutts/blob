
#include "types.h"
#include "blob.h"
#include "vec2d.h"

#include <assert.h>
#include <cmath>
#include <iostream>
#include <ostream>
#include <limits>
#include <list>
#include <math.h>
#include <vector>

using namespace std;

const double TAU = 6.28318530718;

const double EPSILON = 0.001;


vector<spoint> find_hull(vector<spoint> &included, vector<spoint> &excluded)
{
    list<spoint> tmp = giftwrap(included);
    vector<spoint> hull {tmp.begin(), tmp.end()};


    spoint center;
    center.x = 0;
    center.y = 0;
    center.inblob = true; // Not that it should come up.

    for(auto& i : hull) {
        center.x += i.x;
        center.y += i.y;
    }

    center.x /= hull.size();
    center.y /= hull.size();

    // This location of center is just a guess, should work anywhere.
    // We could also use a tree for center, if we tessellate properly,
    // and transverse it clockwise; this might improve the blob placement,
    // but is not required as long as hull is convex.
    // It will really improve cases where the angle subtended by
    // two points is small from the center's perspective.

    // Just to show it visually, TODO remove
    included.push_back(center);


    list<Triangle> triangles = starburst_fix(center, hull, included, excluded);
    hull.clear();
    for(auto& tri : triangles) {
        hull.push_back(tri.b);
    }

    /*
    for(auto& e : excluded) {
        if(point_inside(e, hull)) {
            cerr << "There's a excluded point inside!" << endl;
            cerr << "  " << e << endl;
        }
    }
    */

    return hull;
}

list<Triangle> subdivide_triangle(const Triangle& tri, const spoint& e,
        list<spoint>& interior)
{
    Triangle now_outside(e, tri.b, tri.c);
    for(auto intit = interior.begin(); intit != interior.end(); intit++) {
        auto ico = now_outside.coords(*intit);
        if(ico.u >= 0 && ico.v >= 0 && ico.w >= 0) {
            cerr << "Whoops, excluded some point on the interior"
                << endl << "   " << *intit << endl;
        }
    }
    Triangle before(tri.a, tri.b, e);
    Triangle after (tri.a, e, tri.c);
    return {before, after};
}

list<Triangle> starburst_fix(spoint center, vector<spoint>& hull, vector<spoint>& included, vector<spoint>& excluded)
{
    list<Triangle> triangles;

    for(size_t i_ = 0; i_ < hull.size(); i_++) {
        size_t i = i_ % hull.size();
        size_t j = (i + 1) % hull.size();

        triangles.emplace_back(center, hull[i], hull[j]);
    }

    list<spoint> interior(included.begin(), included.end());

    // Slow :(
    for(auto& p : hull) {
        interior.remove(p);
    }
    list<spoint> excluded_list(excluded.begin(), excluded.end());
    // Filter bounding box



    cerr << "Center: " << center << endl;
    for(auto eit = excluded_list.begin(); eit!= excluded_list.end();) {
        spoint &e = *eit;
        cerr << "Considering point " << e << endl;
        if(abs(e.x) < 0.1 && abs(e.y) < 0.1) {
            cerr << "Excluded point " << e << " near the center" << endl;
        }
        auto trit = triangles.begin();

        bool point_removed = false;

        while(trit != triangles.end()) {
            Triangle::bcoords co = trit->coords(e);
            const float radius_fudge = 0.2;
            if(co.u >= 0 && co.v >= 0  && co.w >= 0) {
                cerr << "Excluded point " << e << " contained in triangle" << endl;
                cerr << "  " << trit->a << ", " << trit->b << ", " << trit->c << endl;
                Triangle tri = *trit;
                trit = triangles.erase(trit);
                triangles.splice(trit, subdivide_triangle(tri, e, interior));

                eit = excluded_list.erase(eit);
                point_removed = true;
                cerr << "After erasing, points left are:" << endl << "    ";
                for(auto& e : excluded_list) {
                    cerr << e;
                }
                cerr << endl;
                break;
            } else if (co.u + radius_fudge >= 0 && co.v >= 0 && co.w >= 0) {
                cerr << "Excluded point " << e << " close to triangle" << endl;
                cerr << "  " << trit->a << ", " << trit->b << ", " << trit->c << endl;
                // TODO better logic?
                Triangle tri = *trit;
                trit = triangles.erase(trit);
                triangles.splice(trit, subdivide_triangle(tri, e, interior));

                eit = excluded_list.erase(eit);
                point_removed = true;
                cerr << "After erasing, points left are:" << endl << "    ";
                for(auto& e : excluded_list) {
                    cerr << e;
                }
                cerr << endl;
                break;

            }
            trit++;
        }
        if (eit == excluded_list.end()) { break; }
        if (!point_removed) {
            eit++;
        }
    }
    return triangles;
}

list<spoint> giftwrap(const vector<spoint> &included) {
    // http://en.wikipedia.org/wiki/Gift_wrapping_algorithm
    list<spoint> inc(included.begin(), included.end());
    if(inc.size() == 2) { return inc; }
    vec2d leftmost{
        numeric_limits<double>::max(),
        numeric_limits<double>::max(),};


    list<spoint> hull;
    // Find a point surely on the convex hull
    list<spoint>::const_iterator leftmost_iter = inc.begin();
    for(auto it = inc.begin(); it != inc.end(); it++) {
        if(it->x <= leftmost.x) {
            if(it->x < leftmost.x || it->y < leftmost.y) {
                leftmost={it->x, it->y};
                leftmost_iter = it;
            }
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
        while(*endit == base){ endit++; }
        auto next = endit; // Will hold our final choice of endpoint


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
            // cerr << "  End: " << *endit << endl;
            // cerr << "    delta_angle: " << delta_angle << endl;
        }
        endit++;

        // cerr << "Base: " << base << endl;


        for(; endit != inc.end(); endit++) {
            if(*endit == base){ endit++; }
            if(endit == inc.end()) break;
            spoint &end = *endit;
            double dx = end.x - base.x;
            double dy = end.y - base.y;
            double distance = dx*dx + dy*dy;

            double angle = atan2(dy, dx);

            double delta_angle = previous_angle - angle;
            while(delta_angle < 0) { delta_angle += TAU; }
            while(delta_angle > TAU) { delta_angle -= TAU; }

            //assert(delta_angle < TAU/2);

            // cerr << "  End: " << end << endl;
            // cerr << "    delta_angle: " << delta_angle << endl;
            double delta_delta_angle = abs(best_delta_angle - delta_angle);
            if( delta_delta_angle < EPSILON ) {
                // If on the same line, pick the closer one
                if(next_distance > distance) {
                    // cerr << "    New closer point" << endl;
                    next = endit;
                    best_delta_angle = delta_angle;
                    next_angle = angle;
                    next_distance = distance;
                }
            } else if(best_delta_angle > delta_angle) {
                // cerr << "    New best angle" << endl;
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
point_inside(const spoint &p, const list<spoint> &points)
{
    bool inside = false;
    spoint e0 = points.back();
    bool y0 = (e0.y > p.y);
    for(auto i = points.begin(); i != points.end(); i++) {
        spoint e1 = *i;
        bool y1 = (e1.y > p.y);
        if(y0 != y1) {
            // Mybe re-read http://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html
            // t == y1 is more efficient maybe, or we can just use t2 instead
            // as it it more obviously correct.  Should benchmark it.
            // bool t = ((e1.y - p.y) * (e0.x - e1.x) >= (e1.x - p.x) * (e0.y - e1.y));
            bool t2 = (p.x < ((e1.x-e0.x) * (p.y-e0.y) / (e1.y-e0.y)) + e0.x);
            if( t2 ) {
                inside = !inside;
                // cerr << "Flipping inside to " << inside << endl;
                // cerr << "  On iteration " << i << endl;
                // cerr << "  Comparing points " << e0 << " and " << e1 << endl;
            }
        }
        e0 = e1;
        y0 = y1;
    }
    return inside;
}


bool point_inside_triangle(
        const spoint& p,
        const spoint& a,
        const spoint& b,
        const spoint& c)
{
    // Stolen from http://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
    // TODO float or double?

    // A cached version lives below

    // Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float v0x = b.x - a.x;
    float v0y = b.y - a.y;
    float v1x = c.x - a.x;
    float v1y = c.y - a.y;
    float v2x = p.x - a.x;
    float v2y = p.y - a.y;

    float d00 = v0x * v0x + v0y * v0y; // Dot(v0, v0)
    float d01 = v0x * v1x + v0y * v1y; // Dot(v0, v1);
    float d11 = v1x * v1x + v1y * v1y; // Dot(v1, v1);
    float d20 = v2x * v0x + v2y * v0y; // Dot(v2, v0);
    float d21 = v2x * v1x + v2y * v1y; // Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    // We can give this fudge factor easily.
    return ( v > 0 && w > 0 && u > 0 &&
             v < 1 && w < 1 && u < 1);
}

// A cached version of the above
Triangle::Triangle(const spoint& a, const spoint& b, const spoint& c) :
    a(a), b(b), c(c),

    v0x(b.x - a.x),
    v0y(b.y - a.y),
    v1x(c.x - a.x),
    v1y(c.y - a.y),
    d00(v0x * v0x + v0y * v0y), // Dot(v0, v0)
    d01(v0x * v1x + v0y * v1y), // Dot(v0, v1)),
    d11(v1x * v1x + v1y * v1y), // Dot(v1, v1)),
    invDenom(1/(d00 * d11 - d01 * d01))
{}

Triangle::bcoords Triangle::coords(const spoint& p) const
{
    float v2x = p.x - a.x;
    float v2y = p.y - a.y;

    float d20 = v2x * v0x + v2y * v0y; // Dot(v2, v0);
    float d21 = v2x * v1x + v2y * v1y; // Dot(v2, v1);
    bcoords co;
    co.v = (d11 * d20 - d01 * d21) * invDenom;
    co.w = (d00 * d21 - d01 * d20) * invDenom;
    co.u = 1.0f - co.v - co.w;
    return co;
}

bool Triangle::contains(const spoint& p) const
{

    bcoords co = coords(p);
    float u = co.u;
    float v = co.v;
    float w = co.w;

    const float radius = 0.2;

    // We can give this fudge factor easily.
    bool contained = ( v > 0 && w > 0 && u > 0 &&
                       v < 1 && w < 1 && u < 1);
    // Fudging
    if(a.inblob) {
        if(! p.inblob) {
            if(b.inblob && c.inblob) {
                contained = v > 0 && w > 0 && u + radius > 0;
            }
        }
    }
    if(contained) {
        cerr << "Contained with co-ords (" << u << ", " << v << ", " << w << ")" << endl;
    }
    return contained;
}


ostream& operator<<(ostream& out, const spoint& p) {
    if(p.inblob) {
        out << "(" << p.x << ", " << p.y << "; " << p.radius << ")";
    } else {
        out << "<" << p.x << ", " << p.y << "; " << p.radius << ">";
    }
    return out;
}
