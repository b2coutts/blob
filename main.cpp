#include "draw.h"
#include "blob.h"
#include "b2.h"
#include "config.h"

#include <sys/time.h>
#include <sys/resource.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

pair<vector<spoint>, vector<spoint> > read_points(istream& in);

pair<vector<spoint>, list< list< vector<size_t> > > >
  read_cook_data(istream &data_file, istream &comb_file);

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
        cerr << "Usage: " << argv[0] << " point_file comb_file output_file" << endl;
        exit(1);
    }

    ifstream data_file, comb_file;

    // TODO: do some error-checking wrt the filename
    curtime("begin");
    data_file.open(argv[1]);
    comb_file.open(argv[2]);
    auto p = read_cook_data(data_file, comb_file);
    vector<spoint> points = p.first;
    list< list< vector< size_t > > > combs = p.second;
    data_file.close();
    comb_file.close();
    curtime("after read");


    const std::vector<color> fill_colors = {
        {255, 102, 0, 77},
        {51, 155, 155,77},
        {0, 0, 153, 77},
        {255, 51, 153,77},
        {102, 51, 255, 77},

        {51, 153, 0,  77},
        {153, 204, 0, 77},
        {102, 0, 102, 77},
        {204, 0,  51, 77},

        {255, 204, 0, 77},
        {255, 0, 0, 77},
        {255, 153, 51, 77},
        {102, 255, 0, 77},
    };

    string combfile_base(argv[3]);
    list< vector< spoint > > comb_hulls;
    int comb_number = -1;
    for(list< vector< size_t > > &comb : p.second) {
        comb_number++;
        curtime("before reseting spoints for new comb");

        comb_hulls.clear();

        int set_number = -1;
        for(vector< size_t > comb_set : comb) {
            set_number ++;
            vector< spoint > included, excluded;
            for(spoint& p : points) {
                p.inblob = false;
            }
            for(size_t i : comb_set) {
                points[i].inblob = true;
            }
            for(spoint& p : points) {
                if(p.inblob) {
                    included.push_back(p);
                } else {
                    excluded.push_back(p);
                }
            }

            list<spoint> fixed = fixed_hull(included, excluded);
            curtime("after fixed_hull");
            cout << "after fixed_hull: "; print_poly(fixed);

            get_radii(fixed, included, excluded);
            curtime("after calculating radii");

            if(RUN_REFINE_POLY){
                refine_poly(fixed, included, excluded);
                curtime("after refine_poly");
                cout << "after refine "; print_poly(fixed);
            }

            if(RUN_RM_CROSSING){
                rm_crossing(fixed, included, excluded);
                curtime("after rm_crossing");
                cout << "after rmcrossing: "; print_poly(fixed);
            }

            vector<spoint> pointvec(begin(fixed), end(fixed));
            comb_hulls.push_back(pointvec);
            curtime("after radii");

            std::stringstream out_filename;
            out_filename << combfile_base << "_" << comb_number
                << "_" << set_number << ".png";
            cerr << "Drawing to " << out_filename.str() << endl;
            draw(OUTPUT_IMG_HEIGHT, OUTPUT_IMG_WIDTH, pointvec, included, excluded,
                    fill_colors[set_number],
                    out_filename.str().c_str());
            curtime("after draw");
            cerr << endl;
        }
        std::stringstream out_filename;
        out_filename << combfile_base << "_" << comb_number << ".png";
        cerr << endl << out_filename.str() << endl;
        draw_many_blobs(OUTPUT_IMG_WIDTH, OUTPUT_IMG_HEIGHT,
                out_filename.str().c_str(),
                points,
                comb_hulls,
                fill_colors);
        cerr << endl;
    }
}
