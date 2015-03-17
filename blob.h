#ifndef BLOB_H
#define BLOB_H

#include "types.h"

#include <vector>

std::vector<spoint>
find_hull(std::vector<spoint> &included,
          std::vector<spoint> &excluded);

bool
point_inside(const spoint &p, const std::vector<spoint> &points);

bool point_inside_triangle(
        const spoint& p,
        const spoint& a,
        const spoint& b,
        const spoint& c);

struct Triangle {
    Triangle(const spoint& a, const spoint& b, const spoint& c);
    const spoint a, b, c;

    bool contains(const spoint& p) const;
    private:
    const float v0x;
    const float v0y;
    const float v1x;
    const float v1y;

    const float d00; // Dot(v0, v0)
    const float d01; // Dot(v0, v1);
    const float d11; // Dot(v1, v1);
    const float invDenom;
};



#endif
