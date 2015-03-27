#include "draw.h"
#include "vec2d.h"
#include "b2.h"

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <math.h>

#include <cairo.h>

using namespace std;
const double TAU = 6.28318530718;
#define PI 3.14159265358979323846

void draw(int width, int height,
        vector<spoint> &hull,
        vector<spoint> &inpoints,
        vector<spoint> &expoints)
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
  scale_world(cr, 2.0, width, height, hull);

  // Actual code call
  //
  cairo_set_line_width(cr, 0.01);
  draw_with_lines(cr, hull);


  cairo_set_line_width(cr, 0.02);
  cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
  draw_with_smoothed_lines(cr, hull, inpoints, expoints);
  draw_points(cr, inpoints, 0.05);

  cairo_set_source_rgba(cr, 0, 0.2, 0.8, 0.6);
  draw_points(cr, expoints, 0.05);

  cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.6);
  draw_axis(cr);

  // End actual code


  cairo_surface_write_to_png(surface, "image.png");

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  return;
}

void scale_world(cairo_t * cr,
        const double boundry,
        const double width, const double height,
        const vector<spoint> &points)
{
  double maxx = numeric_limits<double>::lowest();
  double maxy = numeric_limits<double>::lowest();
  double minx = numeric_limits<double>::max();
  double miny = numeric_limits<double>::max();
  for (auto& s : points) {
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

  // TODO fixed aspect ratio, no smoosh.
  //double scale = min(scalex, scaley);

  cerr << "Scaling by " << scalex << ", " << scaley << endl;
  cairo_scale(cr, scalex, -scaley);
  cairo_translate(cr,
          -minx * ((boundry-1.0) / 2 + 1),
          -maxy * ((boundry-1.0) / 2 + 1));

    // TODO: remove this line; used for a particular debug case
    cairo_translate(cr, 0.3, -0.6);
}

void draw_with_lines(cairo_t *cr, const vector<spoint> &points)
{
    cairo_new_path(cr);
    for(auto& s : points) {
        cerr << "Drawing point " << s << endl;
        cairo_line_to(cr, s.x, s.y);
    }
    cairo_close_path(cr);
    cairo_stroke(cr);
}

void draw_with_smoothed_lines(cairo_t *cr, const vector<spoint> &points,
                             vector<spoint> &inpoints, vector<spoint> &expoints)
{
    cairo_new_path(cr);

    // calculate the radius of each vertex
    // TODO: should this count *all* points of the same inblob type? Are there
    // situations where we can decide we don't need to?
    vector<double> radii(points.size());
    for(int i = 0; i < points.size(); i++){
        radii[i] = numeric_limits<double>::max();
        for(auto &x : inpoints){
            if(x == points[i]) continue;
            radii[i] = min(radii[i], norm(stv(points[i]) - stv(x)));
        }
        for(auto &x : expoints){
            if(x == points[i]) continue;
            radii[i] = min(radii[i], norm(stv(points[i]) - stv(x)));
        }
        // slightly more than 2 to avoid overlap due to imprecision
        radii[i] /= 2.01;
    }

    // TODO: remove this convenient macro
    #define deg(x) (x*360 / TAU)

    // TODO: assumes clockwise convex hull (for last arg)
    double previous_angle;
    pair<double,double> dpair = smooth_line_angle(points.back(),
            *(points.begin()), radii[points.size()-1], radii[0]);
            //rotccw(stv(*(points.begin())) - stv(points.back()) , PI/2));
    previous_angle = dpair.second;

    // TODO draw from last to first

        cout << "cairo_arc(cr, a.x, a.y, a_rad, previous_angle, theta)" << endl;
    for(size_t i = 0; i < points.size(); i++) {
        spoint a = points[i % points.size()];
        spoint b = points[(i + 1) % points.size()];

        double a_rad = radii[i % points.size()];
        double b_rad = radii[(i+1) % points.size()];

        cout << "sla(" << a << ", " << b << ", " << a_rad << ", " << b_rad
             << ", " << ") = ";
        dpair = smooth_line_angle(a, b, a_rad, b_rad);
        cout << "(" << deg(dpair.first) << "," << deg(dpair.second) << ")" << endl;
            //rotccw(stv(*(points.begin())) - stv(points.back()) , PI/2));

        if(a.inblob && b.inblob){
            cout << "NEG(" << a.x << "," << a.y << "," << a_rad << ","
                 << b_rad << "," << deg(previous_angle) << "," << deg(dpair.first) << endl;
            cairo_arc_negative(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }else if(a.inblob && !b.inblob){
            cout << "NEG(" << a.x << "," << a.y << "," << a_rad << ","
                 << b_rad << "," << deg(previous_angle) << "," << deg(dpair.first) << endl;
            cairo_arc_negative(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }else if(!a.inblob && b.inblob){
            cout << "arc(" << a.x << "," << a.y << "," << a_rad << ","
                 << deg(previous_angle) << "," << deg(dpair.first) << endl;
            cairo_arc(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }else{
            cout << "arc(" << a.x << "," << a.y << "," << a_rad << ","
                 << deg(previous_angle) << "," << deg(dpair.first) << endl;
            cairo_arc(cr, a.x, a.y, a_rad, previous_angle, dpair.first);
        }

        previous_angle = dpair.second;
    }
    cairo_close_path(cr);
    cairo_stroke(cr);
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
        cairo_stroke(cr);
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
