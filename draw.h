#ifndef DRAW_H
#define DRAW_H

#include "types.h"

#include <vector>
#include <list>

#include <cairo.h>

struct color {
    color(float r, float g, float b, float a) :
        r(r), g(g), b(b), a(a) {}
    color(int r, int g, int b, int a) :
        r(((float) r)/ 255.0f), g(((float) g)/255.0f), b(((float) b)/ 255.0f),
        a(((float) a)/ 255.0f)
    {}

    float r, g, b, a;
};

struct draw_opts {
    color in_point_color;
    color out_point_color;
    color fill_color;

    draw_opts(color fill) :
        in_point_color(color{1.0f, 0.2f, 0.2f, 0.9f}),
        out_point_color(color{0.0f, 0.2f, 0.8f, 0.9f}),
        fill_color(fill)
        {}
};


void cairo_set_source_color(cairo_t cr, color c);

std::pair<cairo_t*, cairo_surface_t*> draw_init(int width, int height);
void draw_end(cairo_t *cr, cairo_surface_t* surface, const char* filename);

void draw(int width, int height,
        std::vector<spoint> &hull,
        std::vector<spoint> &inpoints,
        std::vector<spoint> &expoints,
        std::vector<double> &radii,
        const color &fill_color,
        const char *filename);

void draw_blob(cairo_t *cr,
        const std::vector<spoint> &hull,
        const std::vector<double> &radii,
        const color &fill_color
        );

void draw_many_blobs(
        int width, int height,
        const char *filename,
        const std::vector<spoint> &points,
        const std::list< std::vector<spoint> > &hulls,
        const std::list< std::vector<double> > &radiii,
        const std::vector< color > &colors
        );

void scale_world(cairo_t * cr,
        double boundry,
        double width, double height,
        const std::vector<spoint> &inpoints,
        const std::vector<spoint> &expoints);
void draw_points(cairo_t *cr, const std::vector<spoint> &points, const double radius);
void draw_with_lines(cairo_t *cr, const std::vector<spoint> &points);
void draw_with_smoothed_lines(cairo_t *cr, const std::vector<spoint> &points,
                             const std::vector<double> &radii);
void draw_axis(cairo_t * cr);

#endif
