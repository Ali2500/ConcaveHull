#include <stdio.h>

#include <exception>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <opencv2/opencv.hpp>

#include "concave_hull_builder.h"

namespace po = boost::program_options;

int main(int argc, char** argv)
{
  po::options_description desc("Concave Hull Builder Options");
  std::string image_path, vertices_file;
  std::vector<int> color;
  bool verbosity = false;

  desc.add_options()
      ("image", po::value<std::string>(&image_path)->required(), "Complete path to image file which will be loaded for drawing on")
      ("vertices", po::value<std::string>(&vertices_file), "Complete path to text file containing list of vertices which will be pre-drawn "
                                               "on the image. Each vertex should be in a separate line in the format \""
                                               "x,y\".")
      ("color", po::value<std::vector<int> >(&color)->multitoken(), "RGB value for the color with which to draw outline of the concave hull."
                                                  "Should be in the format \"R G B\" and in the range [0, 255].")
      ("verbosity,v", "If set, the program displays the complete list of edges each time a new point is added and/or "
                      "alpha is changed")
      ("help,h", "Display help message");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  try
  {
    po::notify(vm);

    if(vm.count("verbosity"))
      verbosity = true;

    if(color.size() == 0)
      color = {0, 255, 0};
    else if(color.size() != 3)
      throw std::runtime_error("The color vector, if provided, must have 3 values in the range [0, 255]");

    if(color[0] < 0 || color[0] > 255 || color[1] < 0 || color[1] > 255 || color[2] < 0 || color[2] > 255)
      throw std::runtime_error("One or more color values were not in the range [0, 255]");
  }
  catch(const std::exception& exc)
  {
    fprintf(stderr, "[ ERROR]: %s\n", exc.what());
    return EXIT_FAILURE;
  }

  if(vm.count("help"))
  {
     std::cout << desc << std::endl;
     return EXIT_SUCCESS;
  }

  cv::Scalar mark_color(color[0], color[1], color[2]);

  cv::Mat image = cv::imread(image_path);

  std::vector<cv::Point2f> points;
  if(!vertices_file.empty())
  {
    std::ifstream readfile(vertices_file);
    std::string line;
    while(std::getline(readfile, line))
    {
      std::vector<std::string> tokens;
      boost::split(tokens, line, boost::is_any_of(", "));
      if(tokens.size() != 2)
        continue;

      try
      {
        points.push_back(cv::Point2f(std::stof(tokens[0]), std::stof(tokens[1])));
      }
      catch(const std::exception& exc)
      {
        continue;
      }
    }

    if(points.empty())
      fprintf(stderr, "[ ERROR]: No points were loaded from the specified vertices file. Maybe it has incorrect formatting.\n");
  }

  int alpha_init_value = std::min(image.cols, image.rows) / 2;

  ConcaveHullBuilder concave_hull_builder;
  concave_hull_builder.setAlpha(alpha_init_value);
  concave_hull_builder.setImage(image);
  concave_hull_builder.setHighlightColor(mark_color);
  concave_hull_builder.setVerbosity(verbosity);

  cv::namedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
  cv::setMouseCallback(WINDOW_NAME, ConcaveHullBuilder::mouseEventCallback, &concave_hull_builder);
  int dummy;
  cv::createTrackbar("Alpha", WINDOW_NAME, &dummy, std::max(image.cols, image.rows), ConcaveHullBuilder::trackbarChangedCallback, &concave_hull_builder);
  cv::setTrackbarPos("Alpha", WINDOW_NAME, alpha_init_value);

  cv::imshow(WINDOW_NAME, image);

  if(!points.empty())
    concave_hull_builder.addPoints(points);

  cv::waitKey();

  return EXIT_SUCCESS;
}
