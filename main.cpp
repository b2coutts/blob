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

int main(int argc, char *argv[]) {
    if(argc <= 2){
        cerr << "Usage: " << argv[0] << " point_file output_file" << endl;
        exit(1);
    }

    ifstream file;

    // TODO: do some error-checking wrt the filename
    curtime("begin");
    file.open(argv[1]);
    auto p = read_points(file);
    file.close();
    curtime("after read");

    list<spoint> fixed = fixed_hull(p.first, p.second);
    curtime("after fixed_hull");

    cout << "before refine "; print_poly(fixed);
    refine_poly(fixed, p.first, p.second);
    curtime("after refine_poly");

    cout << "after refine "; print_poly(fixed);

    vector<spoint> pointvec(begin(fixed), end(fixed));
    vector<double> radii = get_radii(pointvec, p.first, p.second);
    curtime("after radii");

    draw(600, 600, pointvec, p.first, p.second, radii, argv[2]);
    curtime("after draw");
}
