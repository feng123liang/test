// Copyright 2022 Chen Jun
// Licensed under the MIT License.

#ifndef ARMOR_DETECTOR__DETECTOR_HPP_
#define ARMOR_DETECTOR__DETECTOR_HPP_

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

// STD
#include <cmath>
#include <string>
#include <vector>

#include "armor.hpp"

class Detector
{
public:
  struct LightParams
  {
    // width / height
    double min_ratio = 0.1;
    double max_ratio = 0.4;
    // vertical angle
    double max_angle = 40.0;
  };

  struct ArmorParams
  {
    double min_light_ratio = 0.7;
    // light pairs distance
    double min_small_center_distance = 0.8;
    double max_small_center_distance = 3.2;
    double min_large_center_distance = 3.2;
    double max_large_center_distance = 5.5;
    // horizontal angle
    double max_angle = 35.0;
  };

  Detector(const int & bin_thres, const int & color);

  std::vector<Armor> detect(const cv::Mat & input);

  cv::Mat preprocessImage(const cv::Mat & input);
  std::vector<Light> findLights(const cv::Mat & rbg_img, const cv::Mat & binary_img);
  std::vector<Armor> matchLights(const std::vector<Light> & lights);

  // For debug usage
  cv::Mat getAllNumbersImage();
  void drawResults(cv::Mat & img);

  void WholeProcess(cv::Mat & ori_img);

  int binary_thres;
  int detect_color;
  LightParams l;
  ArmorParams a;

  // std::unique_ptr<NumberClassifier> classifier;

  // Debug msgs
  cv::Mat binary_img;
  // auto_aim_interfaces::msg::DebugLights debug_lights;
  // auto_aim_interfaces::msg::DebugArmors debug_armors;

private:
  bool isLight(const Light & possible_light);
  bool containLight(
    const Light & light_1, const Light & light_2, const std::vector<Light> & lights);
  ArmorType isArmor(const Light & light_1, const Light & light_2);

  std::vector<Light> lights_;
  std::vector<Armor> armors_;

  const int Light_draw_strike = 3;
  const int Armor_draw_strike = 5;
};

// namespace rm_auto_aim

#endif  // ARMOR_DETECTOR__DETECTOR_HPP_