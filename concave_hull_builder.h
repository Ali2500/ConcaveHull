#pragma once

#include <atomic>
#include <memory>

#include <opencv2/opencv.hpp>

#define WINDOW_NAME "Concave Hull Builder"
#define SLIDER_INIT_VAL 1000

class ConcaveHullBuilder
{
  public:
    class Edge
    {
      public:
        Edge();
        Edge(const cv::Point2f& p1, const cv::Point2f& p2);
        ~Edge();

        float mag() const;

        cv::Point2f p1() const;
        cv::Point2f p2() const;

        std::string toString() const;

        bool operator== (const Edge& e) const
        {
          return origin.x == e.origin.x && origin.y == e.origin.y && x_offset == e.x_offset && y_offset == e.y_offset;
        }

        bool operator< (const Edge& e) const
        {
          if(origin.x < e.origin.x)
            return true;
          else if(origin.x > e.origin.x)
            return false;

          if(origin.y < e.origin.y)
            return true;
          else if((origin.y > e.origin.y))
            return false;

          if(x_offset < e.x_offset)
            return true;
          else if((x_offset > e.x_offset))
            return false;

          if(y_offset < e.y_offset)
            return true;
          else if((y_offset > e.y_offset))
            return false;

          return false;
        }

        cv::Point2f origin;
        float x_offset, y_offset;
    };
    typedef std::vector<Edge> EdgeList;

    ConcaveHullBuilder();
    ConcaveHullBuilder(int alpha, const cv::Size& image_sz);
    ~ConcaveHullBuilder();

    void setAlpha(int alpha);

    void setImageSize(const cv::Size& image_sz);

    void setHighlightColor(const cv::Scalar& color);

    void setVerbosity(bool verbosity);

    void setImage(cv::Mat& image);

    void addPoint(const cv::Point& pt);

    void addPoints(const std::vector<cv::Point2f>& pts);

    static void mouseEventCallback(int event, int x, int y, int flags, void *user_data);

    static void trackbarChangedCallback(int slider, void* user_data);

  protected:
    void buildConcaveHull();

    int getEdgeLength(const cv::Point2f& p1, const cv::Point2f& p2);

    void drawEdges(const std::vector<cv::Vec6f>& triangles);

    void drawEdges(const EdgeList& edge_list);

    void fillPoints(const EdgeList& edge_list);

    void mouseEventCallback(int event, int x, int y, int flags);

    void trackbarChangedCallback(int slider);

    int alpha_, num_pts_;
    cv::Rect bounding_rect_;
    cv::Subdiv2D div_;
    cv::Mat image_, fresh_image_, binary_image_;
    std::atomic_bool processing_;
    bool fill_hull_, verbosity_;
    int last_slider_val_;
    cv::Scalar highlight_color_;

  public:
    typedef std::shared_ptr<ConcaveHullBuilder> Ptr;
    typedef std::shared_ptr<ConcaveHullBuilder const> ConstPtr;
};
