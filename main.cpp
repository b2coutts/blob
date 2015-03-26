#include "draw.h"
#include "blob.h"
#include "b2.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

pair<vector<spoint>, vector<spoint> > read_points(istream& in);

// flag; if true, use the b2 (chunking) codepath, otherwise use starburst
#define B2 false

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
        vector<spoint> tmp{begin(fixed), end(fixed)};
        points = tmp;
    }

    cout << "Polygon: ";
    for(int i = 0; i < points.size(); i++){
        cout << points[i] << ", ";
    }
    cout << endl;

    /*
    cerr << "Points:" << endl;
    for(auto& p : points) {
        cerr << "  " << p << endl;
    }
    */
    draw(400, 400, points, p.first, p.second);
}
