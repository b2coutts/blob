#include "types.h"

#include <vector>

#include <cairo.h>


void draw(int width, int height, std::vector<spoint> points);
void scale_world(cairo_t * cr,
        double boundry,
        double width, double height,
        const std::vector<spoint> &points);
void draw_with_lines(cairo_t *cr, const std::vector<spoint> &points);
