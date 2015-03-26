#ifndef TYPES_H
#define TYPES_H
#include <ostream>
#include <math.h>

struct spoint {
    spoint(double x, double y, bool inblob) :
        x(x), y(y), inblob(inblob) {};
    spoint() = default;
    ~spoint() = default;
    double x, y;
    bool inblob;
};

inline bool operator==(const spoint &l, const spoint &r) {
    return l.x == r.x && l.y == r.y && l.inblob == r.inblob;
}

inline double dist(spoint a, spoint b){
    return sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) );
}

std::ostream& operator<<(std::ostream& out, const spoint& p);

#endif
