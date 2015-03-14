#include "draw.h"
#include <vector>
using namespace std;

int main() {
    vector<spoint> points;
    points.push_back(spoint(-2, -2, false));
    points.push_back(spoint(0.0, 0.0, true));
    points.push_back(spoint(1.0, 0.0, false));
    points.push_back(spoint(1.0, 1.0, true));
    points.push_back(spoint(0.0, 1.0, true));
    draw(400, 400, points);
}
