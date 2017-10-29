# ConcaveHull
A simple program for drawing and visualizing alpha shapes in images. This is done by performing Delaunay triangulation on the vertices and discarding edges based on the value of alpha. In a nutshell, the method described here is implmented with some alterations: https://stackoverflow.com/questions/23073170/calculate-bounding-polygon-of-alpha-shape-from-the-delaunay-triangulation.

As a top level, the lower the value of alpha, the more tightly the drawn shape will 'hug' the edges between the vertices. As you increase alpha, the drawn shape(s) will become more and more convex. At the maximum value of alpha, you will hbasically have a convex hull of the provided points.

## Build Guide

Just like a typical cmake package. Clone the repository into a directory, cd into it, and then run the following commands in the specified order:

`mkdir build`
`cd build`
`cmake ..`
`make`

If you have the preqrequisite libraries (basically just OpenCV and Boost) installed correctly, the make command should spew out an executable in the build directory that you can then run.

## Usage Guide

Refer to launch options description by running 'ConcaveHull --help'. 

__NOTE:__ Shape filling isn't correctly implemented yet. It's a hacky implementation that only successfully colors the interior of shapes when there are one more vertices 'inside' of it.
