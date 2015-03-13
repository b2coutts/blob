#include <vector>
#include <iostream>

using namespace std;

// point, combined with (allowed in shape)
struct spoint {
    double x,y;
    bool inblob;
};

typedef spline fooooo;

// given a set X of points, computes a path through the vertices of
// conv.hull(X) which does not overlap itself
vector<spoint> ordered_boundary(vector<point> v);

// given a point and a blob (list of vertices), if the point is in the interior
// of blob, add it as a vertex
void cave_poly(spoint p, vector<spoint> v);

int main(){
    unsigned int num_inc, num_exc;
    cin >> num_inc >> num_exc;

    vector<point> inc(num_inc);
    vector<point> exc(num_exc);

    for(int i = 0; i < num_inc; i++){
        cin >> inc[i].x;
        cin >> inc[i].y;
    }

    for(int i = 0; i < num_exc; i++){
        unsigned int tmp;
        cin >> exc[i].x;
        cin >> exc[i].y;
    }

    // TODO: bounding box

    vector<spoint> poly = ordered_boundary(inc);

    for(int i = 0; i < num_exc; i++){
        cave_poly(exc[i], poly);
    }

    // TODO: draw the circle/line curve
    
}
