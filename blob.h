#ifndef BLOB_H
#define BLOB_H

#include "types.h"

#include <vector>
#include <list>


struct Triangle {
    Triangle(const spoint& a, const spoint& b, const spoint& c);
    const spoint a, b, c;

    typedef struct {
        float u, v, w;
    } bcoords;
    // u <=> a,
    bool contains(const spoint& p) const;
    bcoords coords(const spoint& p) const;
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

std::vector<spoint>
find_hull(std::vector<spoint> &included,
          std::vector<spoint> &excluded);
std::list<Triangle>
starburst_fix(spoint center,
              std::vector<spoint>& hull,
              std::vector<spoint>& included,
              std::vector<spoint>& excluded);

bool
point_inside(const spoint &p, const std::list<spoint> &points);

bool point_inside_triangle(
        const spoint& p,
        const spoint& a,
        const spoint& b,
        const spoint& c);

std::list<spoint> giftwrap(std::vector<spoint> &included, std::vector<spoint> &excluded);

#endif
