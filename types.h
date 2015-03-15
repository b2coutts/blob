#ifndef TYPES_H
#define TYPES_H

struct spoint {
    spoint(double x, double y, bool inblob) :
        x(x), y(y), inblob(inblob) {};
    spoint() = default;
    ~spoint() = default;
    double x, y;
    bool inblob;
};

#endif
