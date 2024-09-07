// Copyright (c) 2022 ChenJun
// Licensed under the MIT License.

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
// STD
#include <algorithm>
#include <cmath>
#include <vector>

#include "detector.hpp"

Detector::Detector(
  const int & bin_thres, const int & color)
: binary_thres(bin_thres), detect_color(color){}

std::vector<Armor> Detector::detect(const cv::Mat & input)
{
  binary_img = preprocessImage(input);
  lights_ = findLights(input, binary_img);
  armors_ = matchLights(lights_);

  // if (!armors_.empty()) {
  //   classifier->extractNumbers(input, armors_);
  //   classifier->classify(armors_);
  // }

  return armors_;
}

cv::Mat Detector::preprocessImage(const cv::Mat & rgb_img)
{
  cv::Mat gray_img;
  cv::cvtColor(rgb_img, gray_img, cv::COLOR_RGB2GRAY);

  cv::Mat binary_img;
  cv::threshold(gray_img, binary_img, binary_thres, 255, cv::THRESH_BINARY);

  return binary_img;
}

std::vector<Light> Detector::findLights(const cv::Mat & rbg_img, const cv::Mat & binary_img)
{
  using std::vector;
  vector<vector<cv::Point>> contours;
  vector<cv::Vec4i> hierarchy;
  cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  vector<Light> lights;

  for (const auto & contour : contours) {
    if (contour.size() < 5) continue;

    auto r_rect = cv::minAreaRect(contour);
    auto light = Light(r_rect);

    if (isLight(light)) {
      auto rect = light.boundingRect();
      if (  // Avoid assertion failed
        0 <= rect.x && 0 <= rect.width && rect.x + rect.width <= rbg_img.cols && 0 <= rect.y &&
        0 <= rect.height && rect.y + rect.height <= rbg_img.rows) {
        int sum_r = 0, sum_b = 0;
        auto roi = rbg_img(rect);
        // Iterate through the ROI
        for (int i = 0; i < roi.rows; i++) {
          for (int j = 0; j < roi.cols; j++) {
            if (cv::pointPolygonTest(contour, cv::Point2f(j + rect.x, i + rect.y), false) >= 0) {
              // if point is inside contour
              sum_r += roi.at<cv::Vec3b>(i, j)[0];
              sum_b += roi.at<cv::Vec3b>(i, j)[2];
            }
          }
        }
        // Sum of red pixels > sum of blue pixels ?
        light.color = sum_r > sum_b ? RED : BLUE;
        lights.emplace_back(light);
      }
    }
  }

  return lights;
}

bool Detector::isLight(const Light & light)
{
  // The ratio of light (short side / long side)
  float ratio = light.width / light.length;
  bool ratio_ok = l.min_ratio < ratio && ratio < l.max_ratio;

  bool angle_ok = light.tilt_angle < l.max_angle;

  bool is_light = ratio_ok && angle_ok;

  // Fill in debug information

  return is_light;
}


std::vector<Armor> Detector::matchLights(const std::vector<Light> & lights)
{
  std::vector<Armor> armors;
  

  for (auto light_1 = lights.begin(); light_1 != lights.end(); light_1++) {
    for (auto light_2 = light_1 + 1; light_2 != lights.end(); light_2++) {
  // Loop all the pairing of lights 遍历灯条组，两两匹配，类似小组赛中各队两两比赛
  // 其中.begin()是vector的第一位元素，.end()是vecotr遍历结束的标志
  // auto为自动匹配关键字对应的数据类型，可以是灯条、装甲板等自己预先定义的类
    
      //TODO::在这里实现你对灯条的匹配，从教程种提到的各种“不变条件”进行考虑，计算特征值，
      // 将得到的结果赋值给type: type = ArmorType::INVALID / ArmorType::LARGE / ArmorType::SMALL 即可
      




      auto type; 
      
      if (type != ArmorType::INVALID) {
        auto armor = Armor(*light_1, *light_2);
        armor.type = type;
        armors.emplace_back(armor);
        //将相互匹配、符合要求的灯条记录并添加入armors，遍历完毕后将armors return出去
      }
    }
  }

  return armors;
}

// 可视化，把检测到的灯条和匹配的装甲板画在图片上
void Detector::drawResults(cv::Mat & img)
{
  // Draw Lights
  for (const auto & light : lights_) {
    cv::circle(img, light.top, 3, cv::Scalar(255, 255, 255), Light_draw_strike);
    cv::circle(img, light.bottom, 3, cv::Scalar(255, 255, 255), Light_draw_strike);
    auto line_color = light.color == RED ? cv::Scalar(255, 255, 0) : cv::Scalar(255, 0, 255);
    cv::line(img, light.top, light.bottom, line_color, Light_draw_strike);
  }

  // Draw armors
  for (const auto & armor : armors_) {
    cv::line(img, armor.left_light.top, armor.right_light.bottom, cv::Scalar(0, 255, 0), Armor_draw_strike);
    cv::line(img, armor.left_light.bottom, armor.right_light.top, cv::Scalar(0, 255, 0), Armor_draw_strike);
  }

}

void Detector::WholeProcess(cv::Mat & ori_img){
  binary_img = preprocessImage(ori_img);
  cv::imshow("bin_img", binary_img);
  lights_ = findLights(ori_img, binary_img);
  std::cout << "Lights detected number: " << lights_.size() << std::endl;
  armors_ = matchLights(lights_);
  std::cout << "Armors detected number: " << armors_.size() << std::endl;
  drawResults(ori_img);
}
// namespace rm_auto_aim
