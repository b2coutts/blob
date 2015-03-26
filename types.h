#ifndef TYPES_H
#define TYPES_H
#include <ostream>

struct spoint {
    spoint(double x, double y, bool inblob) :
        x(x), y(y), inblob(inblob), boundary(false) {};
    spoint(double x, double y, bool inblob, bool boundary) :
        x(x), y(y), inblob(inblob), boundary(boundary) {};
    spoint() = default;
    ~spoint() = default;
    double x, y;
    bool inblob;
    bool boundary;
};

inline bool operator==(const spoint &l, const spoint &r) {
    return l.x == r.x && l.y == r.y && l.inblob == r.inblob;
}

std::ostream& operator<<(std::ostream& out, const spoint& p);

#endif
