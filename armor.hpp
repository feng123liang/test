// Copyright 2022 Chen Jun
// Licensed under the MIT License.

#ifndef ARMOR_DETECTOR__ARMOR_HPP_
#define ARMOR_DETECTOR__ARMOR_HPP_

#include <opencv2/core.hpp>

// STL
#include <algorithm>
#include <string>

const int RED = 0;
const int BLUE = 1;

enum class ArmorType { SMALL, LARGE, INVALID }; //规定了Armor类型有三种
const std::string ARMOR_TYPE_STR[3] = {"small", "large", "invalid"};

struct Light : public cv::RotatedRect
{
  Light() = default;
  explicit Light(cv::RotatedRect box) : cv::RotatedRect(box)
  {
    //具体代码理解可参考灯条检测教程
    cv::Point2f p[4];
    box.points(p);
    std::sort(p, p + 4, [](const cv::Point2f & a, const cv::Point2f & b) { return a.y < b.y; });
    top = (p[0] + p[1]) / 2;
    bottom = (p[2] + p[3]) / 2;

    length = cv::norm(top - bottom);
    width = cv::norm(p[0] - p[1]);

    tilt_angle = std::atan2(std::abs(top.x - bottom.x), std::abs(top.y - bottom.y));
    tilt_angle = tilt_angle / CV_PI * 180;
  }

  int color;
  cv::Point2f top, bottom;
  double length;
  double width;
  float tilt_angle;
};

struct Armor
{
  Armor() = default;
  Armor(const Light & l1, const Light & l2)
  {
    // 可以传入两个灯条 来创建Armor对象 Armor armor_name = Armor(l1,l2);
    // 将灯条按照横坐标分为左灯条和右灯条，并计算得到中心点center
    if (l1.center.x < l2.center.x) {
      left_light = l1, right_light = l2;
    } else {
      left_light = l2, right_light = l1;
    }
    center = (left_light.center + right_light.center) / 2;
  }

  // Light pairs part
  Light left_light, right_light;
  cv::Point2f center; // center 为一个平面上的点
  ArmorType type; 

  // Number part
  cv::Mat number_img;
  std::string number;
  float confidence;
  std::string classfication_result;
};

// namespace rm_auto_aim

#endif  // ARMOR_DETECTOR__ARMOR_HPP_
