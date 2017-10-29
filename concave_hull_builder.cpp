#include "concave_hull_builder.h"

#include <math.h>

#include <map>

#include <boost/format.hpp>
#include <boost/functional/hash.hpp>


void printMap(const std::map<ConcaveHullBuilder::Edge, int>& map)
{
  for (auto entry: map)
  {
    printf("%s: %d\n", entry.first.toString().c_str(), entry.second);
  }
}

ConcaveHullBuilder::Edge::Edge()
{
}

ConcaveHullBuilder::Edge::Edge(const cv::Point2f &p1, const cv::Point2f &p2)
{
  cv::Point other_pt;
  if(p1.x < p2.x)
  {
    origin = p1;
    other_pt = p2;
  }
  else if(p1.x > p2.x)
  {
    origin = p2;
    other_pt = p1;
  }

  if(p1.y < p2.y)
  {
    origin = p1;
    other_pt = p2;
  }
  else if(p1.y > p2.y)
  {
    origin = p2;
    other_pt = p1;
  }

  x_offset = other_pt.x - origin.x;
  y_offset = other_pt.y - origin.y;
}

ConcaveHullBuilder::Edge::~Edge()
{
}

float ConcaveHullBuilder::Edge::mag() const
{
  return sqrt(y_offset*y_offset + x_offset*x_offset);
}

cv::Point2f ConcaveHullBuilder::Edge::p1() const
{
  return origin;
}

cv::Point2f ConcaveHullBuilder::Edge::p2() const
{
  cv::Point2f p2;
  p2.x = origin.x + x_offset;
  p2.y = origin.y + y_offset;
  return p2;
}

std::string ConcaveHullBuilder::Edge::toString() const
{
  return boost::str(boost::format("(%d, %d), (%d, %d)") % origin.x % origin.y % p2().x % p2().y);
}

ConcaveHullBuilder::ConcaveHullBuilder() :
  alpha_(INT_MAX),
  num_pts_(0),
  bounding_rect_(0, 0, INT_MAX, INT_MAX),
  processing_(false),
  fill_hull_(true),
  verbosity_(false),
  last_slider_val_(SLIDER_INIT_VAL),
  highlight_color_(0,255,0)
{
}

ConcaveHullBuilder::ConcaveHullBuilder(int alpha, const cv::Size &image_sz) :
  alpha_(alpha),
  num_pts_(0),
  bounding_rect_(0, 0, image_sz.width, image_sz.height),
  processing_(false),
  fill_hull_(true),
  verbosity_(false),
  last_slider_val_(SLIDER_INIT_VAL),
  highlight_color_(0,255,0)
{
}

ConcaveHullBuilder::~ConcaveHullBuilder()
{
}

void ConcaveHullBuilder::setAlpha(int alpha)
{
  alpha_ = alpha;
}

void ConcaveHullBuilder::setImageSize(const cv::Size &image_sz)
{
  bounding_rect_ = cv::Rect(0, 0, image_sz.width, image_sz.height);
  div_ = cv::Subdiv2D(bounding_rect_);
}

void ConcaveHullBuilder::setVerbosity(bool verbosity)
{
  verbosity_ = verbosity;
}

void ConcaveHullBuilder::setHighlightColor(const cv::Scalar &color)
{
  highlight_color_ = color;
}

void ConcaveHullBuilder::setImage(cv::Mat &image)
{
  setImageSize(image.size());
  fresh_image_ = image;
}

void ConcaveHullBuilder::addPoint(const cv::Point &pt)
{
  //highlight point
  cv::Rect rect = cv::Rect(pt, cv::Size(7, 7)) & bounding_rect_;
  cv::Mat roi = fresh_image_(rect);
  roi.setTo(highlight_color_);

  fresh_image_.copyTo(image_);
  div_.insert(pt);
  if(++num_pts_ >= 3)
  {
    buildConcaveHull();
  }

  cv::imshow(WINDOW_NAME, image_);
}

void ConcaveHullBuilder::addPoints(const std::vector<cv::Point2f> &pts)
{
  for(const cv::Point2f pt: pts)
  {
    cv::Rect rect = cv::Rect(pt, cv::Size(7, 7)) & bounding_rect_;
    cv::Mat roi = fresh_image_(rect);
    roi.setTo(highlight_color_);

    div_.insert(pt);
    ++num_pts_;
  }

  fresh_image_.copyTo(image_);
  if(num_pts_ >= 3)
  {
    buildConcaveHull();
  }
  cv::imshow(WINDOW_NAME, image_);
}

void ConcaveHullBuilder::buildConcaveHull()
{
  binary_image_ = cv::Mat(fresh_image_.size(), CV_8U, cv::Scalar(0));
  std::vector<cv::Vec6f> triangles;
  div_.getTriangleList(triangles);

  EdgeList edge_list;

  for(const cv::Vec6f triangle: triangles)
  {
    cv::Point2f p1 = cv::Point2f(triangle[0], triangle[1]);
    cv::Point2f p2 = cv::Point2f(triangle[2], triangle[3]);
    cv::Point2f p3 = cv::Point2f(triangle[4], triangle[5]);

    Edge e1(p1, p2), e2(p2, p3), e3(p1, p3);

    if(bounding_rect_.contains(p1) && bounding_rect_.contains(p2) && bounding_rect_.contains(p3) &&
       e1.mag() <= alpha_ && e2.mag() <= alpha_ && e3.mag() <= alpha_)
    {
      edge_list.push_back(e1);
      edge_list.push_back(e2);
      edge_list.push_back(e3);
    }
  }

  std::map<Edge, int> edge_count;
  for(const Edge& edge: edge_list)
  {
    if(verbosity_)
      printf("Edge: %s\n", edge.toString().c_str());

    if(edge_count.find(edge) == edge_count.end())
      edge_count[edge] = 1;
    else
      ++edge_count[edge];
  }

  edge_list.clear();
  EdgeList interior_edges;
  for(const auto& ec_pair: edge_count)
  {
    if(ec_pair.second == 1)
    {
      edge_list.push_back(ec_pair.first);
    }
    else if(ec_pair.second > 1)
      interior_edges.push_back(ec_pair.first);
  }

  drawEdges(edge_list);

  fillPoints(interior_edges);
}

void ConcaveHullBuilder::mouseEventCallback(int event, int x, int y, int flags)
{
  if(event == CV_EVENT_LBUTTONDOWN)
  {
    addPoint(cv::Point(x, y));
  }
}

void ConcaveHullBuilder::trackbarChangedCallback(int slider)
{
  if(slider < 1 || processing_ || std::abs(last_slider_val_ - slider) < 50)
    return;

  alpha_ = slider;

  if(num_pts_ < 3)
    return;

  processing_ = true;
  last_slider_val_ = slider;
  fresh_image_.copyTo(image_);
  buildConcaveHull();
  cv::imshow(WINDOW_NAME, image_);
  processing_ = false;
}

void ConcaveHullBuilder::trackbarChangedCallback(int slider, void* user_data)
{
  ConcaveHullBuilder* ptr = reinterpret_cast<ConcaveHullBuilder*>(user_data);
  ptr->trackbarChangedCallback(slider);
}

void ConcaveHullBuilder::mouseEventCallback(int event, int x, int y, int flags, void *user_data)
{
  ConcaveHullBuilder* ptr = reinterpret_cast<ConcaveHullBuilder*>(user_data);
  ptr->mouseEventCallback(event, x, y, flags);
}

int ConcaveHullBuilder::getEdgeLength(const cv::Point2f &p1, const cv::Point2f &p2)
{
  float diff_x = p2.x - p1.x, diff_y = p2.y - p1.y;
  return round(sqrt(diff_y*diff_y + diff_x*diff_x));
}

void ConcaveHullBuilder::drawEdges(const std::vector<cv::Vec6f> &triangles)
{
  for(const cv::Vec6f& triangle: triangles)
  {
    cv::line(image_, cv::Point(triangle[0], triangle[1]), cv::Point(triangle[2], triangle[3]), highlight_color_, 1);
    cv::line(image_, cv::Point(triangle[4], triangle[5]), cv::Point(triangle[2], triangle[3]), highlight_color_, 1);
    cv::line(image_, cv::Point(triangle[4], triangle[5]), cv::Point(triangle[0], triangle[1]), highlight_color_, 1);
  }
}

void ConcaveHullBuilder::drawEdges(const EdgeList &edge_list)
{
  for(const Edge& edge: edge_list)
  {
    if(fill_hull_)
    {
      cv::line(image_, edge.p1(), edge.p2(), highlight_color_, 2, CV_AA);
      cv::line(binary_image_, edge.p1(), edge.p2(), cv::Scalar(255), 1);
    }
    else
      cv::line(image_, edge.p1(), edge.p2(), highlight_color_, 2, CV_AA);
  }
}

void ConcaveHullBuilder::fillPoints(const EdgeList &edge_list)
{
  for(const Edge& edge: edge_list)
  {
    cv::floodFill(binary_image_, edge.p2(), cv::Scalar(255));
    cv::floodFill(binary_image_, edge.p1(), cv::Scalar(255));
  }

  cv::Mat pts;
  cv::findNonZero(binary_image_, pts);
  for(std::size_t i = 0; i < pts.total(); ++i)
  {
    cv::Point pt = pts.at<cv::Point>(i);
    image_.at<cv::Vec3b>(pt) = cv::Vec3b(highlight_color_[0], highlight_color_[1], highlight_color_[2]);
  }
}
