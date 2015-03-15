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

    points = find_hull(p.first, p.second);

    draw(400, 400, points);
}
