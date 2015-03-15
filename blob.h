#ifndef BLOB_H
#define BLOB_H

#include "types.h"

#include <vector>

std::vector<spoint>
find_hull(std::vector<spoint> &included,
          std::vector<spoint> &excluded);

bool
point_inside(const std::vector<spoint> &points);



#endif
