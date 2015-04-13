#pragma once
#include <opencv2\opencv.hpp>

void display_mat(cv::Mat texmat, bool fullscreen);

cv::Mat gl_read_color(unsigned int win_width, unsigned int win_height);

cv::Mat gl_read_depth(unsigned int win_width, unsigned int win_height, const cv::Mat& opengl_projection);