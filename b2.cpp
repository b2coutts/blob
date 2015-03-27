#include "types.h"
#include "blob.h"
#include "vec2d.h"
#include "b2.h"

#include <iostream>
#include <algorithm>
#include <limits>

#define PI 3.14159265358979323846

// given a point and a polygon, insert the point between the nearest two
// vertices. Return an iterator to the inserted point
list<spoint>::iterator insert_nearest(const spoint &p, list<spoint> &poly){
    list<spoint>::iterator min_idx = poly.begin();
    list<spoint>::iterator j;
    double min = dist(*min_idx, p) + dist(poly.back(), p);
    cout << "IN: init min: " << min << endl;
    for(j = poly.begin(); j != poly.end(); ++j){
        list<spoint>::iterator next = j; ++next;
        if(next == poly.end()) break;

        double sum = dist(*j, p) + dist(*next, p);
        cout << "IN: (p,*j,*next)=(" << p << "," << *j << "," << *next << ")"
             << ", sum = " << sum << endl;
        if(sum < min){
            cout << "OVERRIDE" << endl;
            min = sum;
            min_idx = next;
        }
    }
    cout << "IN: point=" << p << ", *min_idx=" << *min_idx << endl;

    return poly.insert(min_idx, p);
}

// compute the "fixed" polygon
list<spoint> fixed_hull(vector<spoint> &inc, vector<spoint> &exc){
    list<spoint> hull = giftwrap(inc, exc);
    cout << "initial polygon: ";
    for(auto it = hull.begin(); it != hull.end(); ++it){
        cout << *it << ", ";
    }
    cout << endl;

    for(int i = 0; i < exc.size(); i++){
        if(point_inside(exc[i], hull)){
            cout << "exc point inside: " << exc[i] << endl;
            list<spoint>::iterator before, added, after;
            before = added = after = insert_nearest(exc[i], hull);

            if(before == hull.begin()) before = hull.end();
            --before;
            ++after;
            if(after == hull.end()) after = hull.begin();

            // check if the triangle we removed contained any inc points
            // TODO: can optimize by storing points in kd-tree
            // TODO: does checking every inc point whenever we remove an exc
            //       point take too much time?
            for(int k = 0; k < inc.size(); k++){
                if(point_inside_triangle(inc[k], *before, *added, *after) &&
                   !point_inside(inc[k], hull)){
                    insert_nearest(inc[k], hull);
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
void refine_line(list<spoint> &poly, list<spoint>::iterator ia,
                 list<spoint>::iterator ib, vector<spoint> &inc,
                 vector<spoint> &exc, double *incdists, double *excdists){
    vec2d a = stv(*ia);
    vec2d b = stv(*ib);
    // direction of boundary line a->b
    vec2d dir = b - a;
    // normal vector to dir, pointing outside of poly
    vec2d nrml = rotccw(dir, PI/2); // TODO: requires that &poly is clockwise
    vector<spoint> pts;
    for(int i = 0; i < inc.size(); i++){
        // TODO: efficiency: maybe filter out vertices instead of checking each time
        bool inpoly = false;
        for(auto &pt : poly){
            if(pt == inc[i]) inpoly = true;
        }

        // TODO: we can adjust these thresholds (in particular the dist
        // thresholds) to make it look nicer
        if(!inpoly &&
           inner(nrml, a-stv(inc[i])) < incdists[i] &&
           inner(nrml, a-stv(inc[i])) > 0 &&
           inner(dir, stv(inc[i])-a) > 0 &&
           inner(dir, b-stv(inc[i])) > 0){
            pts.push_back(inc[i]);
        }
    }

    for(int i = 0; i < exc.size(); i++){
        bool inpoly = false;
        for(auto &pt : poly){
            if(pt == exc[i]) inpoly = true;
        }

        // TODO: we can adjust these thresholds (in particular the dist
        // thresholds) to make it look nicer
        if(!inpoly &&
           inner(nrml, stv(exc[i])-a) < excdists[i] &&
           inner(nrml, stv(exc[i])-a) > 0 &&
           inner(dir, stv(exc[i])-a) > 0 &&
           inner(dir, b-stv(exc[i])) > 0){
            pts.push_back(exc[i]);
        }
    }

    if(pts.empty()) return;

    cmp_vec = dir;
    sort(pts.begin(), pts.end(), inner_cmp);
    for(int i = 0; i < pts.size(); i++){
        list<spoint>::iterator iter = poly.insert(ib, pts[i]);
        
        // TODO: remove these lines
        list<spoint>::iterator prev, next;
        prev = next = iter;
        --prev; ++next;
        cout << "inserted " << *iter << " between " << *prev << " and " << *next
            << endl;
        for(auto &pt : poly){
            if(pt == *iter) cout << "MATCH: " << pt << endl;
        }
        // TODO: double-check that this actually works
        refine_line(poly, iter, ib, inc, exc, incdists, excdists);
    }
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

    for(auto i = poly.begin(); i != poly.end(); ++i){
        list<spoint>::iterator next = i; ++next;
        if(next != poly.end()){
            refine_line(poly, i, next, inc, exc, incdists, excdists);
        }
    }

    free(incdists);
    free(excdists);
}


// TODO: replace with a smart radius function
#define radius(x) (0.001)

// normalizes an angle to be within [0,2*PI)
double normalize(double theta){
    while(theta < 0) theta += 2*PI;
    while(theta >= 2*PI) theta-= 2*PI;
    return theta;
}

// for consecutive points sa,sb, with radii ra,rb, (with outward normal vector
// v), calculates the angles (from sa,sb repsectively) at which to draw a
// smooth line from the circle around sa to the circle around sb
pair<double,double> smooth_line_angle(spoint sa, spoint sb, double ra, double rb){
    vec2d a = stv(sa);
    vec2d b = stv(sb);
    vec2d u = b - a;
    double nrm = norm(u);
    vec2d w = scale(1.0/nrm, u); // normalized u
    double delta = (ra - rb)/nrm;
    if(sa.inblob != sb.inblob) delta = (ra + rb)/nrm;

    vec2d v = rotccw(b-a, PI/2);

    cout << "(CALC u = " << u.x << "," << u.y << ")";
    
    // calculate normal vector to line. Recalculate if the line is on the wrong
    // side of the circles
    vec2d c;
    if(sa.inblob)  c = rotccw(w, acos(delta));
    else            c = rotccw(w, 2*PI - acos(delta));
    cout << "(CALC c = " << c.x << "," << c.y << ")";

    double theta = atan2(c.y,c.x);

    return {theta, sa.inblob == sb.inblob ? theta : normalize(theta + PI)};
}
