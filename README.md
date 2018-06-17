# ConcaveHull
A simple program for drawing and visualizing alpha shapes in images. This is done by performing Delaunay triangulation on the vertices and discarding edges based on the value of alpha. In a nutshell, the method described here is implmented with some alterations: https://stackoverflow.com/questions/23073170/calculate-bounding-polygon-of-alpha-shape-from-the-delaunay-triangulation.

To give a top level description, the lower the value of alpha, the more tightly the drawn shape will hug the edges between the vertices. As alpha is increased, the drawn shape(s) will become more and more convex. For very high alpha, the resulting shape will be equivalent to the convex hull of the provided points.

## Build Guide

Just like a typical cmake package. Clone the repository into a directory, cd into it, and then run the following commands in the specified order:

`mkdir build`  
`cd build`  
`cmake ..`  
`make`

If the preqrequisite libraries (OpenCV and Boost) are installed, the make command should generate an executable in the build directory.

## Usage Guide

Refer to launch options description by running 'ConcaveHull --help'. 

__NOTE:__ Shape filling isn't correctly implemented yet. It's a hacky implementation that only successfully colors the interior of shapes when there are one more vertices 'inside' of it.
