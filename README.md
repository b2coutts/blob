CO759 final project, by Bryan Coutts and Theo Belaire

*blob* is a program which draws nice looking blobs around sets of 2D points. It
takes as input a cloud of 2D points, and a subset of them which are to be
included, and draws a blob containing exactly those points. *blob* tries its
best to generate a blob which maximizes the distance of points to its boundary,
and is nice looking. The primary motivation for *blob* is for the visualization
and analysis of Comb Inequalities for the Travelling Salesman Problem.

Check out the [finished images][1].


### Building
You will need [Cairo][cairo] installed to use this tool.
It should be installed in OSX if you have installed any X11 tool,
or you can get it off homebrew.


This hopefully should build without incident, but it's possible
that the headers for cairo aren't found, in which case you must run
a line like the following

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/opt/X11/lib/pkgconfig

to get it set up.

After that, it's just a simple

    make

and it's ready.

### Running

It should be run like

    ./draw <input_points> <input_sets> <output_prefix>

where *input_points* is a file whose first line is a number *n*, followed by
*n* points, represented as newline separated pairs of integers, like so 

    5
    72 24
    49 13
    69 0
    13 7
    74 17

*input_sets* is a file containing any number of combs. The format of a comb is
an integer *m*, followed by *m* lines, each of the form `k a_1 a_2 ... a_k`,
where each a\_i is the index of an included point from *input_points*.

    4
    3  43 58 15
    2  52 43
    2  58 27
    2  15 80
    10

...followed by a dummy value that is unused.


### Configuration

There is a header, `config.h`, which contains many of the configuration
setttings that control the behavoir of our program.


[1]: http://csclub.uwaterloo.ca/~tbelaire/bico/gal/
[cairo]: http://cairographics.org/
