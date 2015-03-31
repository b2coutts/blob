#include "draw.h"
#include "vec2d.h"
#include "b2.h"
#include "config.h"

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <math.h>

#include <cairo.h>

using namespace std;
const double TAU = 6.28318530718;
#define PI 3.14159265358979323846

double avg_scale;

void cairo_set_source_color(cairo_t *cr, color c) {
  cairo_set_source_rgba (cr, c.r, c.g, c.b, c.a);
}


std::pair<cairo_t*, cairo_surface_t*> draw_init(int width, int height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    return {cairo_create(surface), surface};
}
void draw_end(cairo_t *cr, cairo_surface_t* surface, const char* filename) {
  // TODO: error checking on filename
  cairo_surface_write_to_png(surface, filename);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

void draw(int width, int height,
        vector<spoint> &hull,
        vector<spoint> &inpoints,
        vector<spoint> &expoints,
        vector<double> &radii,
        const color& fill_color,
        const char *filename)
{
  cairo_surface_t *surface;
  cairo_t *cr;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 0, 0, 0);
  /*
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 40.0);
  */

  // Set up scale properly
  scale_world(cr, 1.1, width, height, inpoints, expoints);

  // Actual code call
  //
  cairo_set_line_width(cr, POLY_THICKNESS / avg_scale);
  if(DRAW_POLYGON) draw_with_lines(cr, hull);


  cairo_set_line_width(cr, 0.02);
  cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.9);
  if(DRAW_POINTS) draw_points(cr, inpoints, POINT_RADIUS / avg_scale);

  cairo_set_source_rgba(cr, 0, 0.2, 0.8, 0.9);
  if(DRAW_POINTS) draw_points(cr, expoints, POINT_RADIUS / avg_scale);

  if(DRAW_BLOB) {
      cairo_set_source_color(cr, fill_color);
      draw_with_smoothed_lines(cr, hull, radii);
      cairo_fill_preserve(cr);
      cairo_set_source_rgba (cr, fill_color.r, fill_color.g, fill_color.b,
              0.8);
      cairo_stroke(cr);
  }

  cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.6);
  cairo_set_line_width(cr, AXIS_THICKNESS / avg_scale);
  if(DRAW_AXES) draw_axis(cr);

  // End actual code

  draw_end(cr, surface, filename);
  return;
}



void draw_many_blobs(
        int width, int height,
        const char *filename,
        const std::vector<spoint> &points,
        const std::list< std::vector<spoint> > &hulls,
        const std::list< std::vector<double> > &radiii,
        const std::vector< color > &colors
        ) {


    auto p = draw_init(width, height);
    cairo_t *cr = p.first;
    cairo_surface_t * surface = p.second;
    scale_world(cr, 1.1, width, height, points, points);

    cairo_set_line_width(cr, 0.02);

    if(DRAW_POINTS) {
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        draw_points(cr, points, POINT_RADIUS / avg_scale);
    }
    if(DRAW_AXES){
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.6);
        cairo_set_line_width(cr, AXIS_THICKNESS / avg_scale);
        draw_axis(cr);
    }
    auto hull_it = hulls.cbegin();
    auto radiii_it = radiii.cbegin();
    for(size_t i = 0;

        hull_it != hulls.cend() &&
        radiii_it != radiii.cend() &&
        i < colors.size();

        hull_it++, radiii_it++, i++)
    {
        draw_blob(cr, *hull_it, *radiii_it, colors[i]);
    }

    draw_end(cr, surface, filename);
}

void draw_blob(cairo_t *cr,
        const std::vector<spoint> &hull,
        const std::vector<double> &radii,
        const color &fill_color
        )
{
  cairo_set_source_color(cr, fill_color);
  cairo_set_line_width(cr, POLY_THICKNESS / avg_scale);
  if(DRAW_POLYGON){
      draw_with_lines(cr, hull);
  }

  if(DRAW_BLOB) {
      cairo_set_source_color(cr, fill_color);
      draw_with_smoothed_lines(cr, hull, radii);
      cairo_fill_preserve(cr);
      cairo_set_source_rgba (cr, fill_color.r, fill_color.g, fill_color.b,
              0.8);
      cairo_stroke(cr);
  }

  return;
}

void scale_world(cairo_t * cr,
        const double boundry,
        const double width, const double height,
        const vector<spoint> &inpoints,
        const vector<spoint> &expoints)
{
  double maxx = numeric_limits<double>::lowest();
  double maxy = numeric_limits<double>::lowest();
  double minx = numeric_limits<double>::max();
  double miny = numeric_limits<double>::max();
  for (auto& s : inpoints) {
      maxx = max(s.x, maxx);
      maxy = max(s.y, maxy);
      minx = min(s.x, minx);
      miny = min(s.y, miny);
  }
  for (auto& s : expoints) {
      maxx = max(s.x, maxx);
      maxy = max(s.y, maxy);
      minx = min(s.x, minx);
      miny = min(s.y, miny);
  }


  double user_width = maxx - minx;
  double user_height = maxy - miny;

  user_width *= boundry;
  user_height *= boundry;

  double scalex = ((double) width) / user_width;
  double scaley = ((double) height) / user_height;
  avg_scale = (scalex + scaley) / 2;

  // TODO fixed aspect ratio, no smoosh.
  //double scale = min(scalex, scaley);

  cerr << "Scaling by " << scalex << ", " << scaley << endl;
  cairo_scale(cr, scalex, -scaley);
  cerr << "Maxx " << maxx << ", minx " << minx << endl;
  cerr << "Maxy " << maxy << ", miny " << miny << endl;
  cerr << "width " << user_width << ", height " << user_height << endl;
  double x_offset = (user_width - user_width/boundry) / 2;
  // The reason the y offset works is because we have to move down by
  // user_height anyways
  double y_offset = -maxy * ((boundry-1.0) / 2 + 1);
  cerr << "Translated by " <<
          x_offset << ", " << y_offset << endl;
  cairo_translate(cr, x_offset, y_offset);
}

void draw_with_lines(cairo_t *cr, const vector<spoint> &points)
{
    cairo_new_path(cr);
    for(auto& s : points) {
        cairo_line_to(cr, s.x, s.y);
    }
    cairo_close_path(cr);
    cairo_stroke(cr);
}

void draw_with_smoothed_lines(cairo_t *cr, const vector<spoint> &points,
                             const vector<double> &radii)
{
    cairo_new_path(cr);

    /*
    cerr << "Drawing points " << endl;
    for(auto& s : points) {
        cerr << s << ", ";
    }
    cerr << endl << endl;
    */
    // macro for outputting an angle in degrees
    const auto deg = [](double x) { return x*360 / TAU; };

    // TODO: assumes clockwise convex hull (for last arg)
    double previous_angle;
    pair<double,double> dpair = smooth_line_angle(points.back(),
            *(points.begin()), radii[points.size()-1], radii[0]);
            //rotccw(stv(*(points.begin())) - stv(points.back()) , PI/2));
    previous_angle = dpair.second;

    // TODO draw from last to first

    for(size_t i = 0; i < points.size(); i++) {
        spoint a = points[i % points.size()];
        spoint b = points[(i + 1) % points.size()];

        double a_rad = radii[i % points.size()];
        double b_rad = radii[(i+1) % points.size()];

        // cout << "sla(" << a << ", " << b << ", " << a_rad << ", " << b_rad
        //     << ", " << ") = ";
        dpair = smooth_line_angle(a, b, a_rad, b_rad);
        // cout << "(" << deg(dpair.first) << "," << deg(dpair.second) << ")" << endl;

        if(a.inblob){
            cout << "cairo_arc_negative(" << a.x << "," << a.y << "," << a_rad << ","
                 << deg(previous_angle) << "," << deg(dpair.first) << ")" <<  endl;
            cairo_arc_negative(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }else{
            cout << "cairo_arc(" << a.x << "," << a.y << "," << a_rad << ","
                 << deg(previous_angle) << "," << deg(dpair.first) << ")" << endl;
            cairo_arc(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }

        previous_angle = dpair.second;
    }
    cairo_close_path(cr);
}
void draw_with_smoothed_lines_counterclockwise(cairo_t *cr, const vector<spoint> &points)
{
    cairo_new_path(cr);
    const double radius = 0.1;

    double previous_angle = 0.0;

    // TODO draw from last to first

    for(size_t i = 0; i < points.size()+2; i++) {
        spoint a = points[i % points.size()];
        spoint b = points[(i + 1) % points.size()];

        double dx = b.x - a.x;
        double dy = b.y - a.y;

        double angle = atan2(dy, dx);
        angle -=  TAU / 4;

        double delta_angle = previous_angle - angle;
        while(delta_angle < 0) { delta_angle += TAU; }
        while(delta_angle > TAU) { delta_angle -= TAU; }

        cerr << "Drawing line from " << a << " to " << b << endl;
        cerr << "    " << "Angle: " << angle / TAU * 360 << endl;
        cerr << "    " << "Delta: " << delta_angle / TAU  * 360<< endl;
        if(i == 0) {
            //cairo_arc(cr, a.x, a.y, radius,
                    //angle, angle);
        } else if (a.inblob) {
            if(delta_angle < TAU / 2) {

                double midangle = (angle + previous_angle) / 2;
                cairo_arc(cr, a.x, a.y, radius,
                        midangle,midangle);
            } else {
                cairo_arc(cr, a.x, a.y, radius,
                        //        angle, previous_angle);
                    previous_angle, angle);
            }
        } else {

            if(delta_angle < TAU / 2) {
                cairo_arc_negative(cr, a.x, a.y, radius,
                        previous_angle + TAU/2, angle + TAU/2);
            } else {
                double midangle = (angle + previous_angle) / 2 + TAU/2;
                cairo_arc(cr, a.x, a.y, radius,
                        midangle,midangle);
            }
        }

        previous_angle = angle;
    }
    cairo_stroke(cr);
}
void draw_points(cairo_t *cr, const std::vector<spoint> &points, const double radius) {

    for(auto& s : points) {
        cairo_new_path(cr);
        cairo_arc(cr, s.x, s.y, radius, 0, TAU);
        cairo_fill(cr);
    }
}
void draw_axis(cairo_t *cr) {
    cairo_new_path(cr);
    cairo_line_to(cr, 10,0);
    cairo_line_to(cr, -10,0);
    cairo_stroke(cr);
    cairo_new_path(cr);
    cairo_line_to(cr, 0,10);
    cairo_line_to(cr, 0,-10);
    cairo_stroke(cr);
}
