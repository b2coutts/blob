#include "types.h"
#include "blob.h"
#include "vec2d.h"
#include "b2.h"

#include <iostream>

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

// TODO: replace with a smart radius function
#define radius(x) (0.001)

// returns the endpoints of a smooth line between the borders of the circles
// around a and b. a and b lie near a line l of the polygon; v is the normal
// vector to that line pointing out of the polygon
pair<vec2d,vec2d> smooth_line(spoint sa, spoint sb, vec2d v){
    vec2d a = stv(sa);
    vec2d b = stv(sb);
    vec2d u = b - a;
    double nrm = norm(u);
    vec2d w = scale(1.0/nrm, u); // normalized u
    double delta = (radius(a) + radius(b))/nrm;
    if(sa.inblob != sb.inblob) delta = (radius(a) - radius(b))/nrm;
    
    // calculate normal vector to line. Recalculate if the line is on the wrong
    // side of the circles
    vec2d c = rotccw(w, acos(delta));
    if(inner(c,v) < 0) c = rotccw(w, acos(-delta));

    return {a + scale(radius(a), c) , b + scale(radius(b), c)};
}
