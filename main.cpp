#include "draw.h"
#include "blob.h"
#include "b2.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

pair<vector<spoint>, vector<spoint> > read_points(istream& in);

// flag; if true, use the b2 (chunking) codepath, otherwise use starburst
#define B2 true

// helper function for writing a polygon to stdout
void print_poly(list<spoint> poly){
    cout << "Polygon: ";
    for(auto i = poly.begin(); i != poly.end(); ++i){
        cout << *i << ", ";
    }
    cout << endl;
}

int main() {
    ifstream file;
    vector<spoint> points;

    file.open("points.txt");
    auto p = read_points(file);
    file.close();

    cerr << endl << endl;

    // silly hacks here, but this code will be rewritten anyway
    if(!B2) points = find_hull(p.first, p.second);
    else{
        list<spoint> fixed = fixed_hull(p.first, p.second);

        cout << "before refine "; print_poly(fixed);
        
        refine_poly(fixed, p.first, p.second);

        cout << "after refine "; print_poly(fixed);

        vector<spoint> tmp{begin(fixed), end(fixed)};
        points = tmp;
    }
    /*
    cerr << "Points:" << endl;
    for(auto& p : points) {
        cerr << "  " << p << endl;
    }
    */
    draw(600, 600, points, p.first, p.second);
}
