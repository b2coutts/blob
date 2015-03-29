#include "draw.h"
#include "blob.h"
#include "b2.h"

#include <sys/time.h>
#include <sys/resource.h>

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

pair<vector<spoint>, vector<spoint> > read_points(istream& in);

// prints the currently used CPU time
void curtime(const char *desc){
    struct rusage r;

    getrusage(RUSAGE_SELF, &r);
    double t = ((double)r.ru_utime.tv_sec) +
               ((double)r.ru_utime.tv_usec)/1000000.0;
    cout << "CPU time (" << desc << "): " << t << " seconds." << endl;
}

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
    curtime("begin");
    file.open(argv[1]);
    auto p = read_points(file);
    file.close();
    curtime("after read");

    // silly hacks here, but this code will be rewritten anyway
    if(!B2) points = find_hull(p.first, p.second);
    else{
        list<spoint> fixed = fixed_hull(p.first, p.second);
        curtime("after fixed_hull");

        cout << "before refine "; print_poly(fixed);
        
        refine_poly(fixed, p.first, p.second);
        curtime("after refine_poly");

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
    curtime("after draw");
}
