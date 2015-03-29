#include "types.h"

#include <iostream>
#include <vector>

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
