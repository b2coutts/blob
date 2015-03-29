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

int main(int argc, char *argv[]) {
    if(argc <= 2){
        cerr << "Usage: " << argv[0] << " point_file output_file" << endl;
        exit(1);
    }

    ifstream file;
    vector<spoint> points;

    // TODO: do some error-checking wrt the filename
    file.open(argv[1]);
    auto p = read_points(file);
    file.close();

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
    draw(600, 600, points, p.first, p.second, argv[2]);
}
