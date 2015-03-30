#include "types.h"

#include <iostream>
#include <vector>
#include <list>

using namespace std;

pair<vector<spoint>, vector<spoint> >
read_points(istream& in) {

    vector<spoint> included;
    vector<spoint> excluded;

    size_t isize, esize;

    in >> isize >> esize;
    included.reserve(isize);
    excluded.reserve(esize);

    for(size_t i = 0; i < isize; i++) {
        double x, y;
        in >> x >> y;
        included.emplace_back(x, y, true);
    }
    for(size_t i = 0; i < esize; i++) {
        double x, y;
        in >> x >> y;
        excluded.emplace_back(x, y, false);
    }

    pair<vector<spoint>, vector<spoint> > ret (included, excluded);
    return ret;
}


pair<vector<spoint>, list< list< vector<size_t> > > >
read_cook_data(istream &data_file, istream &comb_file) {
    vector<spoint> points;
    list< list<vector<size_t> > >combs;

    size_t num_points;
    data_file >> num_points;

    points.reserve(num_points);

    for(size_t i = 0; i < num_points; i++) {
        double x, y;
        data_file >> x >> y;
        points.emplace_back(x,y, false); // This will be flipped for each comb
    }



    while (true) {
        size_t comb_size;
        list< vector< size_t > > comb;
        comb_file >> comb_size;
        if( comb_file.eof() ) { break; }
        for(size_t i = 0; i < comb_size; i++) {
            size_t set_size;
            vector<size_t> set;
            set.reserve(set_size);
            comb_file >> set_size;
            for(size_t j = 0; j < set_size; j++) {
                size_t index;
                comb_file >> index;
                set.push_back(index);
            }
            comb.push_back(set);
        }
        size_t constraint;
        comb_file >> constraint;
        (void) constraint;
        combs.push_back(comb);
    }

    return {points, combs};
}
