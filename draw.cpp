#include "draw.h"

#include <vector>
#include <iostream>
#include <limits>

#include <cairo.h>

using namespace std;

void draw(int width, int height, vector<spoint> points)
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
  scale_world(cr, 1.2, width, height, points);

  /*
  double x, y;
  x = 0; y = 0;
  cairo_user_to_device(cr, &x, &y);
  cerr << "Point 0,0 maps to ("<<x<<", "<<y<<")" << endl;
  x = 1; y = 1;
  cairo_user_to_device(cr, &x, &y);
  cerr << "Point 1,1 maps to ("<<x<<", "<<y<<")" << endl;
  */
  cairo_new_path(cr);


  cairo_set_line_width(cr, 0.01);
  // Actual code call
  draw_with_lines(cr, points);

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
}

void draw_with_lines(cairo_t *cr, const vector<spoint> &points)
{

    for(auto& s : points) {
        cerr << "Drawing point " << s.x << " " << s.y << endl;
        cairo_line_to(cr, s.x, s.y);
    }
    cairo_close_path(cr);
    cairo_stroke(cr);
}
