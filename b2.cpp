#include "types.h"
#include "blob.h"
#include "vec2d.h"
#include "b2.h"
#include "config.h"

    #include <assert.h>
#include <iostream>
#include <algorithm>
#include <limits>

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
// vertices. Return an iterator to the inserted point
list<spoint>::iterator insert_nearest(const spoint &p, list<spoint> &poly,
                                      const list<spoint>::iterator start,
                                      const list<spoint>::iterator end){
    list<spoint>::iterator min_idx = start;
    list<spoint>::iterator j;
    vec2d nrml = rotccw(stv(*min_idx) - stv(*end), PI/2);
    nrml = scale(1/norm(nrml), nrml);
    double min = abs( inner(nrml, stv(p) - stv(*end)) );
    //cout << "  init min: " << *min_idx << " with OV=" << min << endl;
    for(j = poly.begin(); j != end; ++j){
        list<spoint>::iterator next = j; ++next;

        nrml = rotccw(stv(*next) - stv(*j), PI/2);
        nrml = scale(1/norm(nrml), nrml);
        double obj_val = abs( inner(nrml, stv(p) - stv(*next)) );
        vec2d linedir = stv(*next) - stv(*j);
        if(obj_val < min &&
           inner(linedir, stv(p)-stv(*j)) > 0 &&
           inner(linedir, stv(*next)-stv(p)) > 0){
            min = obj_val;
            min_idx = next;
            //cout << "  WINNER: " << *min_idx << " w/ OV=" << obj_val << endl;
        }
    }

    //cout << "   ACTUAL INSERT: " << p << " into " << *min_idx << endl;

    return poly.insert(min_idx, p);
}

// compute the "fixed" polygon
list<spoint> fixed_hull(vector<spoint> &inc, vector<spoint> &exc){
    list<spoint> hull = giftwrap(inc);
    cout << "after giftwrap: "; print_poly(hull);
    if(!RUN_FIX_HULL) return hull;

    for(int i = 0; i < exc.size(); i++){
        if(point_inside(exc[i], hull)){
            list<spoint>::iterator before, added, after;
            before = added = after = insert_nearest(exc[i], hull, hull.begin(),
                                                    --(hull.end()));

            if(before == hull.begin()) before = hull.end();
            --before;
            ++after;
            if(after == hull.end()) after = hull.begin();

            //cout << "  CHECKING TRIANGLE " << *before << *added << *after << endl;

            // check if the triangle we removed contained any inc points
            // TODO: can optimize by storing points in kd-tree
            // TODO: does checking every inc point whenever we remove an exc
            //       point take too much time?
            for(int k = 0; k < inc.size(); k++){
                if(point_inside_triangle(inc[k], *before, *added, *after) &&
                   !point_inside(inc[k], hull)){
                    //cout << "  INSERTING " << inc[k] << endl;
                    insert_nearest(inc[k], hull, added, after);
                }
            }
        }
    }

    return hull;
}

// comparison vector used for below function
vec2d cmp_vec;

// comparison function; compares by inner product with cmp_vec
bool inner_cmp(spoint u, spoint v){
    return inner(cmp_vec, stv(v)-stv(u)) > 0.0;
}


// refine a single line of a given polygon
// TODO: there's an issue where, if a point is close to two edges, being close
// to an opposing point across one edge will make its disc small on the other
// side as well, even though this is undesirable
// Returns true iff it made a change
bool refine_line(list<spoint> &poly, list<spoint>::iterator ia,
                 list<spoint>::iterator ib, vector<spoint> &inc,
                 vector<spoint> &exc, double *incdists, double *excdists){
    vec2d a = stv(*ia);
    vec2d b = stv(*ib);
    // direction of boundary line a->b
    vec2d dir = b - a;
    // normal vector to dir, pointing outside of poly
    vec2d nrml = rotccw(dir, PI/2); // TODO: requires that &poly is clockwise

    // get the vector which is closest to the line
    int min_idx = -1;
    bool is_inc = true;
    double mindist = numeric_limits<double>::max();
    for(int i = 0; i < inc.size(); i++){
        bool inpoly = false;
        for(auto &pt : poly){
            if(pt == inc[i]) inpoly = true;
        }

        if(!inpoly &&
           inner(dir, stv(inc[i])-a) > 0 &&
           inner(dir, b-stv(inc[i])) > 0){
            double dist = abs( inner(nrml, stv(inc[i]) - a) );
            if(dist < mindist){
                mindist = dist;
                min_idx = i;
            }
        }
    }

    for(int i = 0; i < exc.size(); i++){
        bool inpoly = false;
        for(auto &pt : poly){
            if(pt == exc[i]) inpoly = true;
        }

        if(!inpoly &&
           inner(dir, stv(exc[i])-a) > 0 &&
           inner(dir, b-stv(exc[i])) > 0){
            double dist = abs( inner(nrml, stv(exc[i]) - a) );
            if(dist < mindist){
                mindist = dist;
                min_idx = i;
                is_inc = false;
            }
        }
    }
    if(min_idx == -1) return false;

    spoint p = is_inc ? inc[min_idx] : exc[min_idx];
    double delta = is_inc ? incdists[min_idx] : excdists[min_idx];
    assert(abs(abs(inner(nrml, stv(p)-a)) - mindist) < 0.01);
    if(mindist < delta * REFINE_EPSILON){
        cout << "inserting " << p << " behind " << *ib << ", OV=" << mindist
             << endl;
        poly.insert(ib, p);
        return true;
    }
    return false;
}

// given a fixed polygon, refine each of its lines
void refine_poly(list<spoint> &poly, vector<spoint> &inc, vector<spoint> &exc){
    // calculate minimum distances from each point to a point of opposite
    // inblob type
    // TODO: new or other?
    double *incdists = new double[inc.size()];
    double *excdists = new double[exc.size()];
    double min;

    for(int i = 0; i < inc.size(); i++){
        min = numeric_limits<double>::max();
        for(int j = 0; j < exc.size(); j++){
            double d = dist(inc[i], exc[j]);
            if(d < min) min = d;
        }
        incdists[i] = min;
    }

    for(int i = 0; i < exc.size(); i++){
        min = numeric_limits<double>::max();
        for(int j = 0; j < inc.size(); j++){
            double d = dist(exc[i], inc[j]);
            if(d < min) min = d;
        }
        excdists[i] = min;
    }

    while(true){
        for(auto i = poly.begin(); i != poly.end(); ++i){
            list<spoint>::iterator next = i; ++next;
            bool changed = false;
            if(next != poly.end()){
                changed = refine_line(poly, i, next, inc, exc, incdists,
                                      excdists);
            }else{
                changed = refine_line(poly, i, poly.begin(), inc, exc, incdists,
                                      excdists);
            }
            if(changed) goto restart;
        }

        break;
        restart: true;
    }

    free(incdists);
    free(excdists);
}

// normalizes an angle to be within [0,2*PI)
double normalize(double theta){
    while(theta < -PI) theta += 2*PI;
    while(theta >= PI) theta-= 2*PI;
    return theta;
}

// like smooth_line_angle, but computes the normal vector c
vec2d smooth_line_normal(spoint sa, spoint sb, double ra, double rb){
    vec2d a = stv(sa);
    vec2d b = stv(sb);
    vec2d u = b - a;
    double nrm = norm(u);
    vec2d w = scale(1.0/nrm, u); // normalized u
    double delta = (ra - rb)/nrm;
    if(sa.inblob != sb.inblob) delta = (ra + rb)/nrm;

    vec2d v = rotccw(b-a, PI/2);
    
    // calculate normal vector to line. Recalculate if the line is on the wrong
    // side of the circles
    vec2d c;
    if(sa.inblob)  c = rotccw(w, acos(delta));
    else            c = rotccw(w, 2*PI - acos(delta));

    return c;
}

// calculate the radius of a single vertex
double vertex_radius(spoint v, vector<spoint> &inpoints,
                             vector<spoint> &expoints){
    double rad = numeric_limits<double>::max();
    for(auto &x : inpoints){
        if(x == v) continue;
        rad = min(rad, norm(stv(v) - stv(x)));
    }
    for(auto &x : expoints){
        if(x == v) continue;
        rad = min(rad, norm(stv(v) - stv(x)));
    }
    rad /= MINDIST_RADIUS_FACTOR;
    return rad;
}

// calculate the radius of each vertex
// TODO: should this count *all* points of the same inblob type? Are there
// situations where we can decide we don't need to?
vector <double> get_radii(const vector<spoint> &points, vector<spoint> &inpoints,
                      vector<spoint> &expoints){
    vector<double> radii(points.size());
    for(int i = 0; i < points.size(); i++){
        radii[i] = vertex_radius(points[i], inpoints, expoints);
    }
    return radii;
}

// for consecutive points sa,sb, with radii ra,rb, (with outward normal vector
// v), calculates the angles (from sa,sb repsectively) at which to draw a
// smooth line from the circle around sa to the circle around sb
pair<double,double> smooth_line_angle(spoint sa, spoint sb, double ra, double rb){
    //cout << "INFO: sa,sb,ra,rb are " << sa << sb << ra << rb << endl;
    vec2d c = smooth_line_normal(sa, sb, ra, rb);
    double theta = atan2(c.y,c.x);
    //cout << "INFO: theta,cy,cx are " << theta << "," << c.y << "," << c.x << endl;
    return {theta, sa.inblob == sb.inblob ? theta : normalize(theta + PI)};
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

        double r1 = vertex_radius(lastpt,inc,exc);
        double r2 = vertex_radius(*s,inc,exc);
        double r3 = vertex_radius(*next,inc,exc);

        vec2d c1 = smooth_line_normal(lastpt, *s, r1, r2);
        vec2d c2 = smooth_line_normal(*s, *next, r2, r3);
        
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
