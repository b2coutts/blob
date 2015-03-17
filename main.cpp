#include "draw.h"
#include "blob.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

pair<vector<spoint>, vector<spoint> > read_points(istream& in);

int main() {
    ifstream file;
    vector<spoint> points;

    file.open("points.txt");
    auto p = read_points(file);
    file.close();

    cerr << endl << endl;
    points = find_hull(p.first, p.second);

    /*
    cerr << "Points:" << endl;
    for(auto& p : points) {
        cerr << "  " << p << endl;
    }
    */
    draw(400, 400, points, p.first, p.second);
}
