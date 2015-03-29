#ifndef DRAW_H
#define DRAW_H

#include "types.h"

#include <vector>

#include <cairo.h>


void draw(int width, int height,
        std::vector<spoint> &hull,
        std::vector<spoint> &inpoints,
        std::vector<spoint> &expoints,
        const char *filename);
void scale_world(cairo_t * cr,
        double boundry,
        double width, double height,
        const std::vector<spoint> &points);
void draw_points(cairo_t *cr, const std::vector<spoint> &points, const double radius);
void draw_with_lines(cairo_t *cr, const std::vector<spoint> &points);
void draw_with_smoothed_lines(cairo_t *cr, const std::vector<spoint> &points,
                             std::vector<spoint> &inpoints, std::vector<spoint> &expoints);
void draw_axis(cairo_t * cr);

#endif
