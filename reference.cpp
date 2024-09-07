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
    
      if (light_1->color != detect_color || light_2->color != detect_color) continue;
      //根据颜色进行判断，light_1,light_2是先前所定义的类，->color是访问灯条类的颜色的代码
	    // detect_color为预先设定的值
			
			if (containLight(*light_1, *light_2, lights)) {
      //判断两个灯条中间是否含有灯条（具体代码见下方）
        continue;
      }
			
      auto type = isArmor(*light_1, *light_2);
      //判断两个灯条能否匹配（具体代码见下方）
      
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

//判断两个灯条中间是否含有灯条
bool Detector::containLight(
  const Light & light_1, const Light & light_2, const std::vector<Light> & lights)
{
  
  auto points = std::vector<cv::Point2f>{light_1.top, light_1.bottom, light_2.top, light_2.bottom};
  //light_1.top,light_1.bottom等分别是Point2f（二维中的点）的类型，此处分别提取传入的两个灯条的上顶点和下顶点，共4个点存入points

  auto bounding_rect = cv::boundingRect(points);
    //根据灯条位置拟合矩形（boundingRect根据传入的points中的各个点，返回一个能包含这些点的最小矩形

  for (const auto & test_light : lights) {
  	//遍历所有的灯条

    if (test_light.center == light_1.center || test_light.center == light_2.center) continue;
    	  //排除参与拟合的两个灯条（center为light对象的中心点）
    	  
    if (
      bounding_rect.contains(test_light.top) || bounding_rect.contains(test_light.bottom) ||
      bounding_rect.contains(test_light.center)) {
      return true;
   		//检测拟合矩形内是否含有灯条（根据矩形中有没有包含当前检测灯条的上顶点、中心点或下顶点，若包含则说明用于拟合矩形的两个灯条之间含有其他灯条，不符合装甲板灯条的规则）
    }
    
  }
  
  return false;
  //遍历完成，拟合的矩形中没有发现灯条点，不含灯条则return false
}

ArmorType Detector::isArmor(const Light & light_1, const Light & light_2)
{
  
  float light_length_ratio = light_1.length < light_2.length ? light_1.length / light_2.length
                                                             : light_2.length / light_1.length;
	// 计算灯条长度比（短边：长边）
  
  bool light_ratio_ok = light_length_ratio > a.min_light_ratio;
	// 比较灯条长度比与最小规定的灯条长度比，若大于则light_ratio_ok 为true
	
  float avg_light_length = (light_1.length + light_2.length) / 2;
	// 灯条长度均值, 用于区分大小装甲板
	
  float center_distance = cv::norm(light_1.center - light_2.center) / avg_light_length;
  //norm返回两点间距离，此处计算两个灯条中心连线与灯条平均高度的比值来对映装甲板的长宽比
  
  bool center_distance_ok = (a.min_small_center_distance <= center_distance &&
                             center_distance < a.max_small_center_distance) ||
                            (a.min_large_center_distance <= center_distance &&
                             center_distance < a.max_large_center_distance);
	//center_distance若位于大装甲板的长宽比的范围内或位于小装甲板的长宽比的范围内，均将center_distance_ok 记为true
  
  cv::Point2f diff = light_1.center - light_2.center;
  float angle = std::abs(std::atan(diff.y / diff.x)) / CV_PI * 180;
  bool angle_ok = angle < a.max_angle;
  // 计算灯条中心连线偏角
  // 两点间通过“-”运算，得到坐标相减后的点（相当于得到一个以原点为起点的向量，再根据atan（）返回向量与x轴的角度（弧度制）再通过➗CV_PI*180得到角度值下的角度值）（附图理解）
	
  bool is_armor = light_ratio_ok && center_distance_ok && angle_ok;
	//若三个条件都符合，则is_armor为true
  
  ArmorType type;
  if (is_armor) {
    type = center_distance > a.min_large_center_distance ? ArmorType::LARGE : ArmorType::SMALL;
    //判断具体是大装甲板还是小装甲板
  } else {
    type = ArmorType::INVALID;
  }
  
  return type;
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

  // Show numbers and confidence
  // for (const auto & armor : armors_) {
  //   cv::putText(
  //     img, armor.classfication_result, armor.left_light.top, cv::FONT_HERSHEY_SIMPLEX, 0.8,
  //     cv::Scalar(0, 255, 255), 2);
  // }
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
