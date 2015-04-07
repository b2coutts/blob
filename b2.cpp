
#include "types.h"
#include "blob.h"
#include "vec2d.h"
#include "b2.h"
#include "config.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>

#define PI 3.14159265358979323846

// helper function for writing a polygon to stdout
void print_poly(list<spoint> poly){
    cout << "Polygon: ";
    for(auto i = poly.begin(); i != poly.end(); ++i){
        cout << *i << ", ";
    }
    cout << endl;
}

// given a point and a polygon, insert the point between the nearest two
// vertices. Return an iterator to the inserted point. Searches only within
// [start,end)
// when use_endstart is true, consider also the line from (--end) to start
list<spoint>::iterator insert_nearest(const spoint &p, list<spoint> &poly,
                                      const list<spoint>::iterator start,
                                      const list<spoint>::iterator end,
                                      bool use_endstart){
    list<spoint>::iterator min_idx = start;
    list<spoint>::iterator j;
    vec2d nrml;
    double min = numeric_limits<double>::max();
    for(j = start; j != end; ++j){
        list<spoint>::iterator next = j; ++next;
        if(next == end){
            if(use_endstart) next = start;
            else break;
        }

        nrml = rotccw(stv(*next) - stv(*j), PI/2);
        nrml = scale(1/norm(nrml), nrml);
        double obj_val = abs( inner(nrml, stv(p) - stv(*next)) );
        vec2d linedir = stv(*next) - stv(*j);
        if(obj_val < min &&
           inner(linedir, stv(p)-stv(*j)) > 0 &&
           inner(linedir, stv(*next)-stv(p)) > 0){
            min = obj_val;
            min_idx = next;
        }
    }

    assert(min != numeric_limits<double>::max());

    //auto db_idx = min_idx; if(min_idx == poly.begin()) db_idx = poly.end(); --db_idx;
    //cout << "fixed_hull inserts " << p << " between " << *db_idx << " and " <<
             //*min_idx << endl;

    return poly.insert(min_idx, p);
}

// true iff the given point is a vertex in the polytope
bool is_vtx(spoint p, const list<spoint> &poly){
    for(auto &v : poly){
        if(v == p) return true;
    }
    return false;
}

// removes excluded points from the interior of the polygon by removing triangles
void rm_exc_pts(list<spoint> &hull, vector<spoint> &inc, vector<spoint> &exc){
    for(int i = 0; i < exc.size(); i++){
        if(point_inside(exc[i], hull)){
            list<spoint>::iterator before, added, after;
            if(is_vtx(exc[i], hull)) continue;
            before = added = after = insert_nearest(exc[i], hull, hull.begin(),
                                                    hull.end(), true);

            if(before == hull.begin()) before = hull.end();
            --before;
            ++after;
            if(after == hull.end()) after = hull.begin();

            //cout << "  CHECKING TRIANGLE " << *before << *added << *after << endl;

            // check if the triangle we removed contained any inc points
            // TODO: does checking every inc point whenever we remove an exc
            //       point take too much time?
            for(int k = 0; k < inc.size(); k++){
                if(point_inside_triangle(inc[k], *before, *added, *after) &&
                   !is_vtx(inc[i], hull)){
                    //cout << "  INSERTING " << inc[k] << endl;
                    insert_nearest(inc[k], hull, before, ++after, false);
                    --after;
                }
            }
        }
    }
}

void rm_cont_pts(list<spoint> &hull){
    auto prev = --(hull.end());
    for(auto i = hull.begin(); i != hull.end(); i++){
        if(i->inblob == prev->inblob){
            if(i->radius + dist(stv(*i), stv(*prev)) < prev->radius){
                cout << "ASDF" << endl;
                hull.erase(i);
                rm_cont_pts(hull);
                return;
            }else if(prev->radius + dist(stv(*i), stv(*prev)) < i->radius){
                cout << "ASDF" << endl;
                hull.erase(prev);
                rm_cont_pts(hull);
                return;
            }
        }
        prev = i;
    }
}

// compute the "fixed" polygon
list<spoint> fixed_hull(vector<spoint> &inc, vector<spoint> &exc){
    list<spoint> hull = giftwrap(inc);
    cout << "after giftwrap: "; print_poly(hull);
    if(!RUN_FIX_HULL) return hull;

    rm_exc_pts(hull, inc, exc);

    return hull;
}

// comparison vector used for below function
vec2d cmp_vec;

// comparison function; compares by inner product with cmp_vec
bool inner_cmp(spoint u, spoint v){
    return inner(cmp_vec, stv(v)-stv(u)) > 0.0;
}

// normalizes an angle to be within [0,2*PI)
double normalize(double theta){
    while(theta < -PI) theta += 2*PI;
    while(theta >= PI) theta-= 2*PI;
    return theta;
}

// like smooth_line_angle, but computes the normal vector c
// if sa's circle is contained in sb's circle, returns nan as first coord.
// if sb's circle is contained in sa's circle, returns nan as second coord.
vec2d smooth_line_normal(spoint sa, spoint sb){
    //cout << "SLN(" << sa << ",  " << sb << ")" << endl;
    double ra = sa.radius;
    double rb = sb.radius;
    vec2d a = stv(sa);
    vec2d b = stv(sb);
    vec2d u = b - a;
    double nrm = norm(u);
    vec2d w = scale(1.0/nrm, u); // normalized u
    double delta = (ra - rb)/nrm;
    if(sa.inblob != sb.inblob) delta = (ra + rb)/nrm;

    if(ra + nrm < rb) return {nan(""), 0};
    if(rb + nrm < ra) return {0, nan("")};

    vec2d v = rotccw(b-a, PI/2);
    
    // calculate normal vector to line. Recalculate if the line is on the wrong
    // side of the circles
    vec2d c;
    if(sa.inblob)  c = rotccw(w, acos(delta));
    else            c = rotccw(w, 2*PI - acos(delta));

    assert(c.x == c.x && c.y == c.y); // checks for nan

    return c;
}

// given two line segments a-b, c-d, returns true iff they cross
bool crosses(vec2d a, vec2d b, vec2d c, vec2d d){
    vec2d c1,c2; // normals to a-b, c-d
    c1 = rotccw(b-a, PI/2);
    c2 = rotccw(d-c, PI/2);
    c1 = scale(1/norm(c1), c1);
    c2 = scale(1/norm(c2), c2);

    // perp of c1 onto c2
    vec2d perp = c1 - scale(inner(c1,c2), c2);
    double k2 = inner(c2, c);
    double k1 = (inner(c1, a) - k2*inner(c1,c2)) / inner(c1,perp);

    // intersection of the infinite lines a-b, c-d
    vec2d inter = scale(k1, perp) + scale(k2, c2);

    double ip1 = inner(b-a, inter);
    double ip2 = inner(d-c, inter);

    return inner(b-a, a) < ip1 && ip1 < inner(b-a, b) &&
           inner(d-c, c) < ip2 && ip2 < inner(d-c, d);
}

// return true iff the line segment a-b intersects the border of the polygon.
// Intersection only counts if the lines form a nondegenerate cross
bool cross_poly(spoint a, spoint b, const list<spoint> poly){
    auto prev = --(poly.end());
    for(auto i = poly.begin(); i != poly.end(); ++i){
        if(a == *i || a == *prev || b == *i || b == *prev) continue;
        if(crosses(stv(a), stv(b), stv(*prev), stv(*i))) return true;
        prev = i;
    }
    return false;
}

// calculate the radius of each vertex
// TODO: should this count *all* points of the same inblob type? Are there
// situations where we can decide we don't need to?
void get_radii(list<spoint> &points, vector<spoint> &inpoints,
               vector<spoint> &expoints){
    for(int i = 0; i < inpoints.size(); i++){
        inpoints[i].radius = numeric_limits<double>::max();
        for(auto &x : inpoints){
            if(x == inpoints[i] || (!cross_poly(x,inpoints[i], points) &&
                                    x.inblob == inpoints[i].inblob)) continue;
            inpoints[i].radius = min(inpoints[i].radius, dist(stv(inpoints[i]), stv(x)));
        }
        for(auto &x : expoints){
            if(x == inpoints[i] || (!cross_poly(x,inpoints[i], points) &&
                                    x.inblob == inpoints[i].inblob)) continue;
            inpoints[i].radius = min(inpoints[i].radius, dist(stv(inpoints[i]), stv(x)));
        }
        inpoints[i].radius /= MINDIST_RADIUS_FACTOR;

        cout << "ASSIGN RADIUS: " << inpoints[i] << endl;

        assert(inpoints[i].radius == inpoints[i].radius); // check for nan

        // TODO: lazy expensive pass to populate the radii in the polytope
        for(auto &pt : points){
            if(pt == inpoints[i]) pt.radius = inpoints[i].radius;
        }
    }

    // repeat for expoints
    for(int i = 0; i < expoints.size(); i++){
        expoints[i].radius = numeric_limits<double>::max();
        for(auto &x : inpoints){
            if(x == expoints[i] || (!cross_poly(x,expoints[i], points) &&
                                    x.inblob == expoints[i].inblob)) continue;
            expoints[i].radius = min(expoints[i].radius, dist(stv(expoints[i]), stv(x)));
        }
        for(auto &x : expoints){
            if(x == expoints[i] || (!cross_poly(x,expoints[i], points) &&
                                    x.inblob == expoints[i].inblob)) continue;
            expoints[i].radius = min(expoints[i].radius, dist(stv(expoints[i]), stv(x)));
        }
        expoints[i].radius /= MINDIST_RADIUS_FACTOR;

        // TODO: lazy expensive pass to populate the radii in the polytope
        for(auto &pt : points){
            if(pt == expoints[i]) pt.radius = expoints[i].radius;
        }
    }
}

// for consecutive points sa,sb, with radii ra,rb, (with outward normal vector
// v), calculates the angles (from sa,sb repsectively) at which to draw a
// smooth line from the circle around sa to the circle around sb
pair<double,double> smooth_line_angle(spoint sa, spoint sb){
    //cout << "INFO: sa,sb,ra,rb are " << sa << sb << ra << rb << endl;
    vec2d c = smooth_line_normal(sa, sb);
    double theta = atan2(c.y,c.x);
    //cout << "INFO: theta,cy,cx are " << theta << "," << c.y << "," << c.x << endl;
    return {theta, sa.inblob == sb.inblob ? theta : normalize(theta + PI)};
}

// true iff the circle of a contains the circle of b
bool contains(spoint a, spoint b){
    double d = dist(stv(a), stv(b));
    return a.inblob == b.inblob &&
           (a.radius < b.radius + d ||
            b.radius < a.radius + d);
}

// given a point p which is not a vertex of poly, find the line of poly closest
// to p; return the iterator of the second polytope vertex defining the line,
// and insert it there if it is sufficiently close. Return true iff it was
// inserted somewhere
bool closest_line(list<spoint> &poly, spoint p){
    vec2d v = stv(p);
    double mindist = numeric_limits<double>::max();
    list<spoint>::iterator min_it;
    // find nearest line
    for(auto vtx = poly.begin(); vtx != poly.end(); ++vtx){
        list<spoint>::iterator next = vtx; ++next;
        if(next == poly.end()) next = poly.begin();

        vec2d poly_nrml = rotccw(stv(*next) - stv(*vtx), PI/2);

        vec2d nrml = smooth_line_normal(*vtx, *next);
        //assert(nrml.x == nrml.x && nrml.y == nrml.y);
        vec2d a = stv(*vtx) + scale((*vtx).radius, nrml);
        vec2d b = stv(*next) + scale((*next).radius,
                                ((*vtx).inblob == (*next).inblob ? nrml : -nrml));
        if(inner(nrml, poly_nrml) < 0) nrml = -nrml; // hack for orientation to work
        double dist = inner(nrml, v-a);
        if(-dist > p.radius && p.inblob) continue;
        if(dist < 0 && inner(poly_nrml, v-stv(*next)) < 0 && !p.inblob) continue;
        if(inner(b-a, v-a) < 0 || inner(b-a, v-b) > 0) continue;
        if(dist < mindist){
            mindist = dist;
            min_it = next;
            //cout << "new mindist: p=" << p << ", min_it= " << *min_it
                 //<< ", dist=" << dist << endl;
        }
    }

    if(mindist == numeric_limits<double>::max()) return false;
    
    // insert if sufficiently close
    //cout << "pt is " << p << ", mindist is " << mindist << ", rad is "
         //<< p.radius << endl;
    auto prev = min_it; --prev;
    if(mindist < p.radius && !contains(*prev, p) && !contains(*min_it, p)){
        cout << "INSERTING " << p << " between " << *prev << " and " << *min_it
             << " (val=" << mindist << ")" << endl;
        poly.insert(min_it, p);
        if(contains(p, *prev)) poly.erase(prev);
        if(contains(p, *min_it)) poly.erase(min_it);
        return true;
    }
    return false;
}

int REMOVETHISVAR = 0; // TODO: remove this var!

// given a fixed polygon, refine each of its lines
void refine_poly(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc){
    auto pts = inc;
    for(int i = 0; i < 2; i++){
        for(auto &pt : pts){
            // make sure point is not already in polygon
            bool inpoly = false;
            for(auto &pt2 : poly){
                if(pt == pt2) inpoly = true;
            }
            if(inpoly) continue;

            bool retval = closest_line(poly, pt);
            if(retval){
                // TODO: remove this check!!!
                if(REMOVETHISVAR++ < 9999)
                refine_poly(poly, inc, exc);
                return;
            }
        }
        pts = exc;
    }
}


// TODO: maybe iterate more efficiently, instead of going *all* the way back to
// the beginning every time something is removed
void rm_crossing(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc){
    if(poly.size() == 2) {return;}
    bool removed_pt = false;
    spoint lastpt = poly.back();
    int idx = 0;
    for(auto s = poly.begin(); s != poly.end(); ++s){
        //cout << endl << "rm_crossing on " << *s << endl;
        auto next = s; ++next;
        if(next == poly.end()) next = poly.begin();

        vec2d v1 = stv(lastpt);
        vec2d v2 = stv(*s);
        vec2d v3 = stv(*next);

        double r1 = lastpt.radius;
        double r2 = (*s).radius;
        double r3 = (*next).radius;

        vec2d c1 = smooth_line_normal(lastpt, *s);
        vec2d c2 = smooth_line_normal(*s, *next);
        
        vec2d a = v1 + scale(r1, c1);
        vec2d b = v2 + scale( (lastpt.inblob == (*s).inblob ? r2 : -r2), c1);
        vec2d c = v2 + scale(r2, c2);
        vec2d d = v3 + scale( ((*s).inblob == (*next).inblob ? r3 : -r3), c2);

        //cout << "a,b,c,d: " << a << b << c << d << endl;

        // perp of c1 onto c2
        vec2d perp = c1 - scale(inner(c1,c2), c2);
        // coefficient of c2
        double k2 = inner(c2, c);
        // coefficient of perp
        double k1 = (inner(c1, a) - k2*inner(c1,c2)) / inner(c1,perp); 

        // intersection of lines a->b and c->d
        vec2d inter = scale(k1, perp) + scale(k2, c2);

        //cout << "inter: " << inter << endl;

        double ip1 = inner(b-a, inter);
        double ip2 = inner(d-c, inter);
        if(inner(b-a, a) < ip1 && ip1 < inner(b-a, b) &&
           inner(d-c, c) < ip2 && ip2 < inner(d-c, d)){
            //cout << "REMOVING " << *s << endl;
            poly.erase(s);
            removed_pt = true;
            //cout << "CUT!" << endl;
            //for(auto &debug : poly) cout << debug << endl;
            break;
        }
        lastpt = *s;
    }

    if(removed_pt) rm_crossing(poly, inc, exc);
}
