CO759 final project, by Bryan Coutts and Theo Belaire


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

where *input_points* is a file consisting of
*n* points, which are newline separated pairs of integers, like so

    5
    72 24
    49 13
    69 0
    13 7
    74 17

And *input_sets* is a file containing any number of combs that look like

    4
    3  43 58 15
    2  52 43
    2  58 27
    2  15 80
    10

First, the number of sets, followed by some sets, then a final number, which
we ignore.
Each set first states its size, then contains that many indecies into the
point list.


### Configuration

There is a header, `config.h`, which contains many of the configuration
setttings that control the behavoir of our program.


[1]: http://csclub.uwaterloo.ca/~tbelaire/bico/gal/
[cairo]: http://cairographics.org/
