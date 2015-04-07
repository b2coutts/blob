// defines several configurable parameters for the program
#ifndef CONFIG_H
#define CONFIG_H

/* CONFIG PARAMS FOR MAIN.CPP */
// toggles whether or not to run fix_hull, refine_poly, rm_crossing
#define RUN_FIX_HULL true
#define RUN_REFINE_POLY true
#define RUN_RM_CROSSING true

// width and height of output image
#define OUTPUT_IMG_WIDTH 600
#define OUTPUT_IMG_HEIGHT 600


/* CONFIG PARAMS FOR DRAW.CPP */
// scales the size of drawn points
#define POINT_RADIUS 2.0

// scales the thickness of the drawn polytope
#define POLY_THICKNESS 0.6

// scales the thickness of the drawn axes
#define AXIS_THICKNESS 0.6

// toggles whether or not to draw the axes, radii, points, polygon, blob
#define DRAW_AXES false
#define DRAW_RADII false
#define DRAW_POINTS true
#define DRAW_POLYGON true
#define DRAW_BLOB true
#define DRAW_BACKGROUND true

// controls buffer around edge of screen; larger => more space
#define EDGE_BUFFER_WIDTH 1.5


/* CONFIG PARAMS FOR B2.CPP */
// factor by which one divides the minimum distance to get the radius.
// Increasing this number will make the blob smaller/thinner.  Must be at least
// 2.0. 3.0 seems optimal
#define MINDIST_RADIUS_FACTOR 3.0

// points for which min_dist_to_other_point*REFINE_EPSILON > dist_to_line will
// be added to the polytope. REFINE_EPSILON should be at least 1.0; increasing
// it causes the polytope to reach further for vertices
#define REFINE_EPSILON 1.0

#endif
